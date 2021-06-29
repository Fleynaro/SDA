package sda.sync.mappers;

import ghidra.program.model.data.DataType;
import ghidra.program.model.data.TypeDef;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeTypedef;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.IMapper;
import sda.sync.SyncContext;

public class TypedefTypeMapper implements IMapper {

    private Sda sda;
    public DataTypeMapper dataTypeMapper;

    public TypedefTypeMapper(Sda sda, DataTypeMapper dataTypeMapper) {
        this.sda = sda;
        this.dataTypeMapper = dataTypeMapper;
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for(SDataTypeTypedef typedefDesc : dataPacket.getTypedefs()) {
            DataType type = dataTypeMapper.findDataTypeByGhidraId(typedefDesc.getType().getId());
            changeTypedefByDesc((TypeDef)type, typedefDesc);
        }
    }

    public void upsert(SyncContext ctx, TypeDef type) {
        ctx.dataPacket.getTypedefs().add(buildDesc(type));
        dataTypeMapper.upsert(ctx, type);
    }

    private SDataTypeTypedef buildDesc(TypeDef typeDef) {
        SDataTypeTypedef typedefDesc = new SDataTypeTypedef();
        typedefDesc.setType(dataTypeMapper.buildDesc(typeDef));
        typedefDesc.setRefType(dataTypeMapper.buildTypeUnitDesc(typeDef.getDataType()));
        return typedefDesc;
    }

    private void changeTypedefByDesc(TypeDef typeDef, SDataTypeTypedef typedefDesc) {
        dataTypeMapper.changeTypeByDesc(typeDef, typedefDesc.getType());
        //typeDef.dataTypeReplaced(typeDef.getDataType(), dataTypeMapper.getTypeByDesc(typedefDesc.getRefType()));
    }
}
