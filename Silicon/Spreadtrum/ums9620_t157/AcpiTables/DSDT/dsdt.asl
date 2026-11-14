DefinitionBlock("DSDT.aml", "DSDT", 0x02, "SPRDD ", "UMS512  ", 1)
{
    Scope (\_SB)
    {
        Include ("addSub.asl")
        Include ("dsdt_common.asl")
    }
}
