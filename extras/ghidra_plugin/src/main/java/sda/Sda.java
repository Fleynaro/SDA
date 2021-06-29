package sda;

import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Program;
import sda.sync.Sync;

public class Sda {
    public static String dataTypeCategory = "SDA";

    private Program program;
    private Sync sync;

    Sda(Program program)
    {
        this.program = program;
        sync = new Sync(this);
    }

    private long getBaseAddress() {
        return getProgram().getAddressMap().getImageBase().getOffset();
    }

    public long getOffsetByAddress(Address address) {
        return address.getOffset() - getBaseAddress();
    }

    public Address getAddressByOffset(long offset) {
        return getProgram().getAddressMap().getImageBase().getNewAddress(getBaseAddress() + offset);
    }

    public Program getProgram() {
        return program;
    }

    public Sync getSync() {
        return sync;
    }
}
