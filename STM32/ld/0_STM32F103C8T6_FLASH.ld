/* Specify the memory areas */
MEMORY
{
    RAM(xrw)    : ORIGIN = 0x20000000, LENGTH = 20k

    /* BOOT(xr)	: ORIGIN = 0x08000000, LENGTH = 16k */
    PROG(xr)	: ORIGIN = 0x08000000, LENGTH = 64k
}

REGION_ALIAS("TEXT", PROG);
REGION_ALIAS("LOAD", PROG);
