/** @file
  SprdTouchDxe.c

  Spreadtrum UMS512 Synaptics/S3908 touch DXE driver.

  Copyright (c) 2026, hxzbaka. All rights reserved.<BR>

  SPDX-License-Identifier: GPL-2.0-only
**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

#include <Protocol/AbsolutePointer.h>

#include "SprdUms512Tcm.h"

STATIC SPRD_TCM_CONTEXT* mTcmContext;

#define SPRD_TOUCH_BUILD_ID  "S3908-TCM-ACTIVELOW-POLL-r9-20260805"

STATIC
EFI_STATUS
TcmReadBits(
    IN CONST UINT8* Payload,
    IN UINTN PayloadLength,
    IN UINTN BitsOffset,
    IN UINTN BitsToRead,
    OUT UINT32* Value
)
{
    UINTN Remaining;
    UINTN BitOffset;
    UINTN ByteOffset;
    UINTN Available;
    UINTN Take;
    UINT8 Byte;
    UINT32 Result;

    if (Payload == NULL || Value == NULL || BitsToRead == 0U ||
        BitsToRead > 32U || BitsOffset > PayloadLength * 8U ||
        BitsToRead > (PayloadLength * 8U - BitsOffset)) {
        return EFI_INVALID_PARAMETER;
    }

    Remaining = BitsToRead;
    BitOffset = BitsOffset % 8U;
    ByteOffset = BitsOffset / 8U;
    Result = 0U;
    while (Remaining != 0U) {
        Byte = (UINT8)(Payload[ByteOffset] >> BitOffset);
        Available = 8U - BitOffset;
        Take = MIN(Available, Remaining);
        Byte &= (UINT8)(0xffU >> (8U - Take));
        Result |= (UINT32)Byte << (BitsToRead - Remaining);
        Remaining -= Take;
        ByteOffset++;
        BitOffset = 0U;
    }
    *Value = Result;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
TcmReadFieldWidth(
    IN CONST UINT8* Config,
    IN UINTN ConfigLength,
    IN OUT UINTN* ConfigIndex,
    OUT UINTN* BitsToRead
)
{
    if (Config == NULL || ConfigIndex == NULL || BitsToRead == NULL ||
        *ConfigIndex >= ConfigLength) {
        return EFI_INVALID_PARAMETER;
    }
    *BitsToRead = Config[(*ConfigIndex)++];
    if (*BitsToRead > 255U) {
        return EFI_INVALID_PARAMETER;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
TcmReadMessage(
    IN OUT SPRD_TCM_CONTEXT* Context,
    OUT UINT8* Code,
    OUT UINT8* Payload,
    IN UINTN PayloadCapacity,
    OUT UINTN* PayloadLength
)
{
    EFI_STATUS Status;
    TCM_MESSAGE_HEADER Header;
    UINT8* Continuation;
    UINTN ContinuationLength;
    UINTN BodyLength;

    if (Context == NULL || Code == NULL || Payload == NULL ||
        PayloadLength == NULL || PayloadCapacity == 0U) {
        return EFI_INVALID_PARAMETER;
    }

    *PayloadLength = 0U;
    Status = SprdUms512I2cTransfer(
        &Context->I2c, TRUE, (UINT8*)&Header, NULL,
        sizeof(Header)
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }
    if (Header.Marker != TCM_MESSAGE_MARKER ||
        Header.Length > TCM_MAX_MESSAGE_SIZE) {
        DEBUG((EFI_D_WARN, "SynapticsTCM: invalid header marker=0x%02x length=%u\n",
            Header.Marker, Header.Length));
        return EFI_PROTOCOL_ERROR;
    }

    *Code = Header.Code;
    if (Header.Length == 0U) {
        return EFI_SUCCESS;
    }
    if (Header.Length > PayloadCapacity) {
        return EFI_BUFFER_TOO_SMALL;
    }

    ContinuationLength = (UINTN)Header.Length + 3U;
    Continuation = AllocateZeroPool(ContinuationLength);
    if (Continuation == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = SprdUms512I2cTransfer(
        &Context->I2c, TRUE, Continuation, NULL, ContinuationLength
    );
    if (EFI_ERROR(Status)) {
        FreePool(Continuation);
        return Status;
    }
    if (Continuation[0] != TCM_MESSAGE_MARKER ||
        Continuation[1] != TCM_STATUS_CONTINUED_READ ||
        Continuation[ContinuationLength - 1U] != TCM_MESSAGE_PADDING) {
        DEBUG((EFI_D_WARN,
            "SynapticsTCM: invalid continuation marker=0x%02x code=0x%02x pad=0x%02x\n",
            Continuation[0], Continuation[1],
            Continuation[ContinuationLength - 1U]));
        FreePool(Continuation);
        return EFI_PROTOCOL_ERROR;
    }

    /* The continuation header is two bytes and the last byte is padding. */
    BodyLength = ContinuationLength - 3U;
    CopyMem(Payload, Continuation + 2U, BodyLength);
    if (BodyLength != Header.Length) {
        FreePool(Continuation);
        return EFI_PROTOCOL_ERROR;
    }
    *PayloadLength = BodyLength;
    FreePool(Continuation);
    return EFI_SUCCESS;
}

EFI_STATUS
TcmWriteCommand(
    IN OUT SPRD_TCM_CONTEXT* Context,
    IN UINT8 Command,
    IN CONST UINT8* Payload OPTIONAL,
    IN UINTN PayloadLength
)
{
    UINT8* Buffer;
    UINTN Length;
    EFI_STATUS Status;

    if (Context == NULL || PayloadLength > (TCM_MAX_MESSAGE_SIZE - 3U) ||
        (PayloadLength != 0U && Payload == NULL)) {
        return EFI_INVALID_PARAMETER;
    }

    /* S3908 command frames are command plus a little-endian payload length. */
    Length = 3U + PayloadLength;
    Buffer = AllocateZeroPool(Length);
    if (Buffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }
    Buffer[0] = Command;
    Buffer[1] = (UINT8)(PayloadLength & 0xffU);
    Buffer[2] = (UINT8)(PayloadLength >> 8U);
    if (PayloadLength != 0U) {
        CopyMem(Buffer + 3U, Payload, PayloadLength);
    }

    Status = SprdUms512I2cTransfer(&Context->I2c, FALSE, NULL, Buffer, Length);
    FreePool(Buffer);
    return Status;
}

EFI_STATUS
TcmWaitForInterrupt(
    IN SPRD_TCM_CONTEXT* Context,
    IN UINT32 TimeoutUs
)
{
    EFI_STATUS Status;
    BOOLEAN Level;
    UINT32 Elapsed;

    if (Context == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    Status = SprdUms512GpioInput(&Context->Gpio, SPRD_UMS512_TCM_IRQ_GPIO);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Elapsed = 0U;
    while (Elapsed < TimeoutUs) {
        Status = SprdUms512GpioGet(&Context->Gpio, SPRD_UMS512_TCM_IRQ_GPIO, &Level);
        if (EFI_ERROR(Status)) {
            return Status;
        }
        if ((SPRD_UMS512_TCM_IRQ_ACTIVE_LOW != 0 && !Level) ||
            (SPRD_UMS512_TCM_IRQ_ACTIVE_LOW == 0 && Level)) {
            return EFI_SUCCESS;
        }
        gBS->Stall(1000U);
        Elapsed += 1000U;
    }
    return EFI_TIMEOUT;
}

EFI_STATUS
TcmGetReportConfig(
    IN OUT SPRD_TCM_CONTEXT* Context
)
{
    EFI_STATUS Status;
    UINT8 Code;
    UINTN Length;
    STATIC CONST UINT8 S3908PartPrefix[] = { 's', '3', '9', '0', '8' };
    STATIC CONST UINT8 S3908DefaultReportConfig[] = {
      TOUCH_FRAME_RATE, 8U,
      0x1eU, 8U,
      TOUCH_0D_BUTTONS_STATE, 1U,
      TOUCH_PAD_TO_NEXT_BYTE,
      TOUCH_FOREACH_ACTIVE_OBJECT,
      TOUCH_OBJECT_N_INDEX, 4U,
      TOUCH_OBJECT_N_CLASSIFICATION, 4U,
      TOUCH_OBJECT_N_X_POSITION, 12U,
      TOUCH_OBJECT_N_Y_POSITION, 12U,
      TOUCH_OBJECT_N_Z, 8U,
      TOUCH_OBJECT_N_X_WIDTH, 8U,
      TOUCH_OBJECT_N_Y_WIDTH, 8U,
      TOUCH_OBJECT_N_TX_POSITION_TIXELS, 16U,
      TOUCH_OBJECT_N_RX_POSITION_TIXELS, 16U,
      TOUCH_FOREACH_END,
      TOUCH_END
    };

    Status = TcmWriteCommand(Context, TCM_CMD_GET_TOUCH_REPORT_CONFIG, NULL, 0U);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    Status = TcmWaitForInterrupt(Context, TCM_COMMAND_TIMEOUT_US);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    Status = TcmReadMessage(
        Context, &Code, Context->ReportConfig.Data,
        sizeof(Context->ReportConfig.Data), &Length
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }
    if (Code == TCM_STATUS_NOT_IMPLEMENTED) {
        if (CompareMem(Context->Identify.PartNumber, S3908PartPrefix,
            sizeof(S3908PartPrefix)) != 0) {
            DEBUG((EFI_D_ERROR, "SynapticsTCM: touch report config unsupported for this IC\n"));
            return EFI_UNSUPPORTED;
        }
        CopyMem(Context->ReportConfig.Data, S3908DefaultReportConfig,
            sizeof(S3908DefaultReportConfig));
        Context->ReportConfig.Length = sizeof(S3908DefaultReportConfig);
        DEBUG((EFI_D_INFO, "SynapticsTCM: using S3908 default touch report config\n"));
        return EFI_SUCCESS;
    }
    if (Code > TCM_STATUS_ERROR || Length == 0U) {
        DEBUG((EFI_D_WARN, "SynapticsTCM: report config response code=0x%02x\n", Code));
        return EFI_DEVICE_ERROR;
    }
    Context->ReportConfig.Length = Length;
    return EFI_SUCCESS;
}

EFI_STATUS
TcmParseTouchReport(
    IN SPRD_TCM_CONTEXT* Context,
    IN CONST UINT8* Payload,
    IN UINTN PayloadLength,
    OUT TCM_TOUCH_DATA* Touch
)
{
    CONST UINT8* Config;
    UINTN ConfigLength;
    UINTN ConfigIndex;
    UINTN BitsOffset;
    UINTN BitsToRead;
    UINTN LoopStart;
    UINT32 Value;
    UINT32 ActiveCount;
    UINT32 Iteration;
    BOOLEAN HaveActiveCount;
    BOOLEAN InLoop;
    BOOLEAN ActiveOnly;
    BOOLEAN ObjectActive;
    BOOLEAN SkipObjectPayload;
    BOOLEAN HaveFirst;
    EFI_STATUS Status;

    if (Context == NULL || Payload == NULL || Touch == NULL ||
        Context->ReportConfig.Length == 0U) {
        return EFI_INVALID_PARAMETER;
    }

    ZeroMem(Touch, sizeof(*Touch));
    Config = Context->ReportConfig.Data;
    ConfigLength = Context->ReportConfig.Length;
    ConfigIndex = 0U;
    BitsOffset = 0U;
    LoopStart = 0U;
    ActiveCount = 0U;
    Iteration = 0U;
    HaveActiveCount = FALSE;
    InLoop = FALSE;
    ActiveOnly = FALSE;
    ObjectActive = FALSE;
    SkipObjectPayload = FALSE;
    HaveFirst = FALSE;

    while (ConfigIndex < ConfigLength) {
        UINT8 Field = Config[ConfigIndex++];

        switch (Field) {
        case TOUCH_END:
            return EFI_SUCCESS;

        case TOUCH_FOREACH_ACTIVE_OBJECT:
        case TOUCH_FOREACH_OBJECT:
            LoopStart = ConfigIndex;
            Iteration = 0U;
            InLoop = TRUE;
            ActiveOnly = (Field == TOUCH_FOREACH_ACTIVE_OBJECT);
            /* An active-object loop contains only active contacts. */
            ObjectActive = ActiveOnly;
            SkipObjectPayload = ActiveOnly && HaveActiveCount && (ActiveCount == 0U);
            break;

        case TOUCH_FOREACH_END:
            if (!InLoop) {
                return EFI_PROTOCOL_ERROR;
            }
            if (ActiveOnly) {
                if ((!HaveActiveCount && BitsOffset < PayloadLength * 8U) ||
                    (HaveActiveCount && ActiveCount != 0U &&
                        Iteration + 1U < ActiveCount &&
                        Iteration + 1U < TCM_MAX_FINGERS)) {
                    Iteration++;
                    ConfigIndex = LoopStart;
                    ObjectActive = TRUE;
                }
                else {
                    InLoop = FALSE;
                    SkipObjectPayload = FALSE;
                }
            }
            else if (Iteration + 1U < TCM_MAX_FINGERS) {
                Iteration++;
                ConfigIndex = LoopStart;
                ObjectActive = FALSE;
            }
            else {
                InLoop = FALSE;
                SkipObjectPayload = FALSE;
            }
            break;

        case TOUCH_PAD_TO_NEXT_BYTE:
            BitsOffset = (BitsOffset + 7U) & ~7U;
            break;

        case TOUCH_CUSTOMER_GESTURE_INFO:
        case TOUCH_CUSTOMER_GESTURE_INFO2:
            Status = TcmReadFieldWidth(
                Config, ConfigLength, &ConfigIndex, &BitsToRead
            );
            if (EFI_ERROR(Status)) {
                return Status;
            }
            BitsOffset += BitsToRead;
            break;

        default:
            Status = TcmReadFieldWidth(
                Config, ConfigLength, &ConfigIndex, &BitsToRead
            );
            if (EFI_ERROR(Status)) {
                return Status;
            }
            if (SkipObjectPayload) {
                break;
            }
            if (BitsToRead == 0U) {
                break;
            }
            if (Field == TOUCH_NUM_OF_ACTIVE_OBJECTS) {
                Status = TcmReadBits(
                    Payload, PayloadLength, BitsOffset, BitsToRead, &Value
                );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                ActiveCount = MIN(Value, (UINT32)TCM_MAX_FINGERS);
                HaveActiveCount = TRUE;
            }
            else if (Field == TOUCH_OBJECT_N_CLASSIFICATION) {
                Status = TcmReadBits(
                    Payload, PayloadLength, BitsOffset, BitsToRead, &Value
                );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                ObjectActive = (Value != 0U);
            }
            else if (Field == TOUCH_OBJECT_N_X_POSITION) {
                Status = TcmReadBits(
                    Payload, PayloadLength, BitsOffset, BitsToRead, &Value
                );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                if (ObjectActive && !HaveFirst) {
                    Touch->X = (UINT16)MIN(Value, 0xffffU);
                }
            }
            else if (Field == TOUCH_OBJECT_N_Y_POSITION) {
                Status = TcmReadBits(
                    Payload, PayloadLength, BitsOffset, BitsToRead, &Value
                );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                if (ObjectActive && !HaveFirst) {
                    Touch->Y = (UINT16)MIN(Value, 0xffffU);
                    Touch->Active = TRUE;
                    HaveFirst = TRUE;
                }
            }
            BitsOffset += BitsToRead;
            break;
        }
    }

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
TcmReadIdentify(
    IN OUT SPRD_TCM_CONTEXT* Context
)
{
    EFI_STATUS Status;
    UINT8 Code;
    UINTN Length;

    Status = TcmReadMessage(
        Context, &Code, Context->Message.Data,
        sizeof(Context->Message.Data), &Length
    );
    if (EFI_ERROR(Status) || Code != TCM_REPORT_IDENTIFY) {
        Status = TcmWriteCommand(
            Context, TCM_CMD_IDENTIFY, NULL, 0U
        );
        if (EFI_ERROR(Status)) {
            return Status;
        }
        Status = TcmWaitForInterrupt(Context, TCM_COMMAND_TIMEOUT_US);
        if (EFI_ERROR(Status)) {
            return Status;
        }
        Status = TcmReadMessage(
            Context, &Code, Context->Message.Data,
            sizeof(Context->Message.Data), &Length
        );
    }
    if (EFI_ERROR(Status) || Code != TCM_REPORT_IDENTIFY ||
        Length < sizeof(Context->Identify)) {
        return EFI_DEVICE_ERROR;
    }
    CopyMem(&Context->Identify, Context->Message.Data, sizeof(Context->Identify));
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
TcmAbsolutePointerReset(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL* This,
    IN BOOLEAN ExtendedVerification
)
{
    SPRD_TCM_CONTEXT* Context;

    (VOID)ExtendedVerification;
    Context = mTcmContext;
    if (Context == NULL || This != &Context->AbsolutePointer) {
        return EFI_INVALID_PARAMETER;
    }
    ZeroMem(&Context->AbsolutePointerState, sizeof(Context->AbsolutePointerState));
    Context->StateChanged = FALSE;
    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
TcmAbsolutePointerGetState(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL* This,
    OUT EFI_ABSOLUTE_POINTER_STATE* State
)
{
    SPRD_TCM_CONTEXT* Context;

    Context = mTcmContext;
    if (Context == NULL || State == NULL || This != &Context->AbsolutePointer) {
        return EFI_INVALID_PARAMETER;
    }
    if (!Context->StateChanged) {
        return EFI_NOT_READY;
    }
    CopyMem(State, &Context->AbsolutePointerState, sizeof(*State));
    Context->StateChanged = FALSE;
    return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
TcmAbsolutePointerWaitForInput(
    IN EFI_EVENT Event,
    IN VOID* ContextPointer
)
{
    SPRD_TCM_CONTEXT* Context = (SPRD_TCM_CONTEXT*)ContextPointer;

    if (Context != NULL && Context->StateChanged) {
        gBS->SignalEvent(Event);
    }
}

VOID
EFIAPI
TcmPollCallback(
    IN EFI_EVENT Event,
    IN VOID* ContextPointer
)
{
    SPRD_TCM_CONTEXT* Context;
    BOOLEAN Level;
    BOOLEAN Asserted;
    EFI_STATUS Status;
    UINT8 Code;
    UINTN Length;
    TCM_TOUCH_DATA Touch;
    BOOLEAN PointerStateChanged;

    Context = (SPRD_TCM_CONTEXT*)ContextPointer;
    (VOID)Event;
    if (Context == NULL || Context->Exiting || Context->PollInProgress) {
        return;
    }
    Context->PollInProgress = TRUE;

    Status = SprdUms512GpioGet(
        &Context->Gpio, SPRD_UMS512_TCM_IRQ_GPIO, &Level
    );
    if (EFI_ERROR(Status)) {
        goto Exit;
    }
    Asserted = (SPRD_UMS512_TCM_IRQ_ACTIVE_LOW != 0) ? !Level : Level;
    if (!Asserted || !Context->Initialized) {
        goto Exit;
    }

    Status = TcmReadMessage(
        Context, &Code, Context->Message.Data,
        sizeof(Context->Message.Data), &Length
    );
    if (EFI_ERROR(Status)) {
        if (Context->DiagnosticMessages < 4U) {
            DEBUG((EFI_D_WARN, "SprdTouch: read failed: %r\n", Status));
            Context->DiagnosticMessages++;
        }
        Context->FailureCount++;
        goto Exit;
    }
    if (Code != TCM_REPORT_TOUCH) {
        goto Exit;
    }

    Status = TcmParseTouchReport(
        Context, Context->Message.Data, Length, &Touch
    );
    if (EFI_ERROR(Status)) {
        if (Context->DiagnosticMessages < 4U) {
            DEBUG((EFI_D_WARN, "SprdTouch: touch parse failed: %r len=%u\n",
                Status, (UINT32)Length));
            Context->DiagnosticMessages++;
        }
        Context->FailureCount++;
        goto Exit;
    }

    PointerStateChanged = (Touch.Active != Context->LastActive) ||
        (Touch.Active && (Touch.X != Context->LastX ||
            Touch.Y != Context->LastY));
    if (Touch.Active) {
        if (!Context->LastActive) {
            DEBUG((EFI_D_INFO, "SprdTouch: DOWN x=%u y=%u\n", Touch.X, Touch.Y));
        }
        else if (Touch.X != Context->LastX || Touch.Y != Context->LastY) {
            DEBUG((EFI_D_INFO, "SprdTouch: MOVE x=%u y=%u\n", Touch.X, Touch.Y));
        }
        Context->LastX = Touch.X;
        Context->LastY = Touch.Y;
    }
    else if (Context->LastActive) {
        DEBUG((EFI_D_INFO, "SprdTouch: UP x=%u y=%u\n",
            (UINT32)Context->LastX, (UINT32)Context->LastY));
    }
    if (PointerStateChanged) {
        Context->AbsolutePointerState.CurrentX = Touch.Active ? Touch.X : Context->LastX;
        Context->AbsolutePointerState.CurrentY = Touch.Active ? Touch.Y : Context->LastY;
        Context->AbsolutePointerState.CurrentZ = 0U;
        Context->AbsolutePointerState.ActiveButtons =
            Touch.Active ? EFI_ABSP_TouchActive : 0U;
        Context->StateChanged = TRUE;
    }
    Context->LastActive = Touch.Active;
    Context->FailureCount = 0U;

Exit:
    if (Context->FailureCount >= TCM_MAX_FAILURES_BEFORE_RESET &&
        !Context->Exiting) {
        Context->FailureCount = 0U;
        (VOID)SprdUms512TouchReset(&Context->Gpio);
    }
    Context->PollInProgress = FALSE;
}

STATIC
VOID
EFIAPI
TcmExitBootServices(
    IN EFI_EVENT Event,
    IN VOID* ContextPointer
)
{
    SPRD_TCM_CONTEXT* Context = (SPRD_TCM_CONTEXT*)ContextPointer;

    (VOID)Event;

    if (Context == NULL) {
        return;
    }
    Context->Exiting = TRUE;
    if (Context->PollTimer != NULL) {
        gBS->SetTimer(Context->PollTimer, TimerCancel, 0U);
    }
}

STATIC
VOID
TcmFreeContext(
    IN SPRD_TCM_CONTEXT* Context
)
{
    if (Context == NULL) {
        return;
    }
    if (mTcmContext == Context) {
        mTcmContext = NULL;
    }
    if (Context->PollTimer != NULL) {
        gBS->CloseEvent(Context->PollTimer);
    }
    if (Context->ExitBootServices != NULL) {
        gBS->CloseEvent(Context->ExitBootServices);
    }
    if (Context->AbsolutePointer.WaitForInput != NULL) {
        gBS->CloseEvent(Context->AbsolutePointer.WaitForInput);
    }
    if (Context->ProtocolInstalled) {
        gBS->UninstallProtocolInterface(
            Context->Handle, &gEfiAbsolutePointerProtocolGuid,
            &Context->AbsolutePointer
        );
    }
    FreePool(Context);
}

EFI_STATUS
EFIAPI
SprdTouchDxeInitialize(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
)
{
    EFI_STATUS Status;
    SPRD_TCM_CONTEXT* Context;

    (VOID)ImageHandle;
    (VOID)SystemTable;

    if (mTcmContext != NULL) {
        return EFI_ALREADY_STARTED;
    }
    DEBUG((EFI_D_INFO, "SprdTouch: build=" SPRD_TOUCH_BUILD_ID "\n"));
    Context = AllocateZeroPool(sizeof(*Context));
    if (Context == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = SprdUms512GpioInit(&Context->Gpio);
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = SprdUms512I2cInit(&Context->I2c);
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = SprdUms512GpioInput(&Context->Gpio, SPRD_UMS512_TCM_IRQ_GPIO);
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = SprdUms512TouchReset(&Context->Gpio);
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = TcmReadIdentify(Context);
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "SynapticsTCM: identify failed: %r\n", Status));
        goto Error;
    }
    Status = TcmGetReportConfig(Context);
    if (EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "SynapticsTCM: report config failed: %r\n", Status));
        goto Error;
    }
    Context->AbsolutePointer.Reset = TcmAbsolutePointerReset;
    Context->AbsolutePointer.GetState = TcmAbsolutePointerGetState;
    Context->AbsolutePointer.Mode = &Context->AbsolutePointerMode;
    Context->AbsolutePointerMode.AbsoluteMinX = 0U;
    Context->AbsolutePointerMode.AbsoluteMinY = 0U;
    Context->AbsolutePointerMode.AbsoluteMinZ = 0U;
    Context->AbsolutePointerMode.AbsoluteMaxX = SPRD_UMS512_TCM_MAX_X;
    Context->AbsolutePointerMode.AbsoluteMaxY = SPRD_UMS512_TCM_MAX_Y;
    Context->AbsolutePointerMode.AbsoluteMaxZ = 0U;
    Context->AbsolutePointerMode.Attributes = 0U;
    Status = gBS->CreateEvent(
        EVT_NOTIFY_WAIT, TPL_NOTIFY, TcmAbsolutePointerWaitForInput,
        Context, &Context->AbsolutePointer.WaitForInput
    );
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = gBS->CreateEvent(
        EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK, TcmPollCallback,
        Context, &Context->PollTimer
    );
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Status = gBS->SetTimer(
        Context->PollTimer, TimerPeriodic, TCM_POLL_PERIOD_100NS
    );
    if (EFI_ERROR(Status)) {
        goto Error;
    }

    Status = gBS->CreateEventEx(
        EVT_NOTIFY_SIGNAL, TPL_CALLBACK, TcmExitBootServices, Context,
        &gEfiEventExitBootServicesGuid, &Context->ExitBootServices
    );
    if (EFI_ERROR(Status)) {
        goto Error;
    }

    Context->Initialized = TRUE;
    mTcmContext = Context;
    Status = gBS->InstallProtocolInterface(
        &Context->Handle, &gEfiAbsolutePointerProtocolGuid,
        EFI_NATIVE_INTERFACE, &Context->AbsolutePointer
    );
    if (EFI_ERROR(Status)) {
        goto Error;
    }
    Context->ProtocolInstalled = TRUE;
    return EFI_SUCCESS;

Error:
    TcmFreeContext(Context);
    return Status;
}