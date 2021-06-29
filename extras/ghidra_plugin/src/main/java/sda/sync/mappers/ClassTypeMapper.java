package sda.sync.mappers;

import ghidra.program.model.data.DataType;
import ghidra.program.model.data.Structure;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeClass;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.IMapper;
import sda.sync.SyncContext;

public class ClassTypeMapper implements IMapper {

    private Sda sda;
    public StructureTypeMapper structureTypeMapper;

    public ClassTypeMapper(Sda sda, StructureTypeMapper structureTypeMapper) {
        this.sda = sda;
        this.structureTypeMapper = structureTypeMapper;
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for(SDataTypeClass classDesc : dataPacket.getClasses()) {
            DataType type = structureTypeMapper.dataTypeMapper.findDataTypeByGhidraId(classDesc.getStructType().getType().getId());
            changeClassByDesc((Structure)type, classDesc);
        }
    }

    public void upsert(SyncContext ctx, Structure type) {
        ctx.dataPacket.getClasses().add(buildDesc(type));
        structureTypeMapper.dataTypeMapper.upsert(ctx, type);
    }

    private SDataTypeClass buildDesc(Structure Class) {
        SDataTypeClass ClassDesc = new SDataTypeClass();
        ClassDesc.setStructType(structureTypeMapper.buildDesc(Class));

        return ClassDesc;
    }

    private void changeClassByDesc(Structure Class, SDataTypeClass classDesc) {
        structureTypeMapper.changeStructureByDesc(Class, classDesc.getStructType());

    }
}
