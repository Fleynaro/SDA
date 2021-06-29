package sda.sync.mappers;

import ghidra.program.database.function.OverlappingFunctionException;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.SourceType;
import ghidra.util.exception.DuplicateNameException;
import ghidra.util.exception.InvalidInputException;
import sda.Sda;
import sda.ghidra.datatype.SFunctionArgument;
import sda.ghidra.function.SFunction;
import sda.ghidra.function.SFunctionRange;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.ghidra.variable.SGlobalVar;
import sda.sync.IBaseMapper;
import sda.sync.SyncContext;

import java.util.List;

public class GlobalVarMapper implements IBaseMapper {
    private Sda sda;
    public Listing listing;
    private DataTypeMapper dataTypeMapper;

    public GlobalVarMapper(Sda sda, Listing listing, DataTypeMapper dataTypeMapper) {
        this.sda = sda;
        this.listing = listing;
        this.dataTypeMapper = dataTypeMapper;
    }

    @Override
    public void loadToRemove(SDataFullSyncPacket dataPacket) {
        for(Long id : dataPacket.removed_globalVars) {
            Data data = findDataByGhidraId(id);
            if(data != null) {
                data.clearAllSettings();
            }
        }
    }

    @Override
    public void loadToCreate(SDataFullSyncPacket dataPacket) {
        for (SGlobalVar gvarDesc : dataPacket.globalVars) { }
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for (SGlobalVar gvarDesc : dataPacket.globalVars) {
            Data data = findDataByGhidraId(gvarDesc.getId());
            if(data != null) {
                changeDataByDesc(data, gvarDesc);
            }
        }
    }

    public void upsert(SyncContext ctx, Data data) {
        ctx.dataPacket.globalVars.add(buildDesc(data));
    }

    private void changeDataByDesc(Data data, SGlobalVar gvarDesc) {
        data.setComment(0, gvarDesc.getComment());

    }

    private long getGhidraId(Data data) {
        return sda.getOffsetByAddress(data.getAddress());
    }

    private SGlobalVar buildDesc(Data data) {
        SGlobalVar gvarDesc = new SGlobalVar();
        gvarDesc.setId(getGhidraId(data));
        gvarDesc.setName(data.getFieldName());
        gvarDesc.setComment(data.getComment(0));
        gvarDesc.setType(dataTypeMapper.buildTypeUnitDesc(data.getDataType()));
        return gvarDesc;
    }

    private Data findDataByGhidraId(long id) {
        Address addr = sda.getAddressByOffset(id);
        return listing.getDefinedDataAt(addr);
    }
}
