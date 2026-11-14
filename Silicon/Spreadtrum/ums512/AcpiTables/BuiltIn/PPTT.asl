/* UMS512: one package, six Cortex-A55 CPUs, and two Cortex-A75 CPUs. */
[0004] Signature : "PPTT"
[0004] Table Length : 00000100
[0001] Revision : 01
[0001] Checksum : 00
[0006] Oem ID : "SPRD  "
[0008] Oem Table ID : "UMS512  "
[0004] Oem Revision : 00000001
[0004] Asl Compiler ID : "INTL"
[0004] Asl Compiler Revision : 20200925

/* Package at 0x24. */
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 00000001
Physical package : 1
ACPI Processor ID valid : 0
Processor is a thread : 0
Node is a leaf : 0
Identical Implementation : 0
[0004] Parent : 00000000
[0004] ACPI Processor ID : 00000000
[0004] Private Resource Number : 00000000

/* A55 cluster at 0x38. */
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 00000000
Physical package : 0
ACPI Processor ID valid : 0
Processor is a thread : 0
Node is a leaf : 0
Identical Implementation : 0
[0004] Parent : 00000024
[0004] ACPI Processor ID : 00000000
[0004] Private Resource Number : 00000000

/* A55 cores 0-5, parent 0x38. */
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000000
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000001
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000002
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000003
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000004
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 00000038
[0004] ACPI Processor ID : 00000005
[0004] Private Resource Number : 00000000

/* A75 cluster at 0xC4. */
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 00000000
Physical package : 0
ACPI Processor ID valid : 0
Processor is a thread : 0
Node is a leaf : 0
Identical Implementation : 0
[0004] Parent : 00000024
[0004] ACPI Processor ID : 00000000
[0004] Private Resource Number : 00000000

/* A75 cores 6-7, parent 0xC4. */
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 000000C4
[0004] ACPI Processor ID : 00000006
[0004] Private Resource Number : 00000000
[0001] Subtable Type : 00
[0001] Length : 14
[0002] Reserved : 0000
[0004] Flags (decoded below) : 0000000A
Physical package : 0
ACPI Processor ID valid : 1
Processor is a thread : 0
Node is a leaf : 1
Identical Implementation : 0
[0004] Parent : 000000C4
[0004] ACPI Processor ID : 00000007
[0004] Private Resource Number : 00000000
