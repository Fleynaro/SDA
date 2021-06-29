package sda.sync.mappers;

import ghidra.program.model.data.DataType;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeEnum;
import sda.ghidra.datatype.SDataTypeEnumField;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.IMapper;
import sda.sync.SyncContext;

import java.util.ArrayList;

public class EnumTypeMapper implements IMapper {

    private Sda sda;
    public DataTypeMapper dataTypeMapper;

    public EnumTypeMapper(Sda sda, DataTypeMapper dataTypeMapper) {
        this.sda = sda;
        this.dataTypeMapper = dataTypeMapper;
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for(SDataTypeEnum enumDesc : dataPacket.getEnums()) {
            DataType type = dataTypeMapper.findDataTypeByGhidraId(enumDesc.getType().getId());
            changeEnumByDesc((ghidra.program.model.data.Enum)type, enumDesc);
        }
    }

    public void upsert(SyncContext ctx, ghidra.program.model.data.Enum type) {
        ctx.dataPacket.getEnums().add(buildDesc(type));
        dataTypeMapper.upsert(ctx, type);
    }

    private SDataTypeEnum buildDesc(ghidra.program.model.data.Enum enumeration) {
        SDataTypeEnum enumDesc = new SDataTypeEnum();
        enumDesc.setType(dataTypeMapper.buildDesc(enumeration));

        String[] fields = enumeration.getNames();
        enumDesc.setFields(new ArrayList<>());
        for(String fieldName : fields) {
            SDataTypeEnumField enumField = new SDataTypeEnumField();
            enumField.setName(fieldName);
            enumField.setValue((int)enumeration.getValue(fieldName));
            enumDesc.addToFields(enumField);
        }
        return enumDesc;
    }

    private void changeEnumByDesc(ghidra.program.model.data.Enum enumeration, SDataTypeEnum enumDesc) {
        dataTypeMapper.changeTypeByDesc(enumeration, enumDesc.getType());

        for(String fieldName : enumeration.getNames()) {
            enumeration.remove(fieldName);
        }

        for(SDataTypeEnumField field : enumDesc.getFields()) {
            enumeration.add(field.getName(), field.getValue());
        }
    }
}
