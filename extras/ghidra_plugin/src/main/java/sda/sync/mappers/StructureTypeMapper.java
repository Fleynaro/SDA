package sda.sync.mappers;

import ghidra.program.model.data.*;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeStructure;
import sda.ghidra.datatype.SDataTypeStructureField;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.IMapper;
import sda.sync.SyncContext;

import java.util.ArrayList;

public class StructureTypeMapper implements IMapper {

    private Sda sda;
    public DataTypeMapper dataTypeMapper;

    public StructureTypeMapper(Sda sda, DataTypeMapper dataTypeMapper) {
        this.sda = sda;
        this.dataTypeMapper = dataTypeMapper;
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for(SDataTypeStructure structureDesc : dataPacket.getStructures()) {
            DataType type = dataTypeMapper.findDataTypeByGhidraId(structureDesc.getType().getId());
            changeStructureByDesc((Structure)type, structureDesc);
        }
    }

    public void upsert(SyncContext ctx, Structure type) {
        ctx.dataPacket.getStructures().add(buildDesc(type));
        dataTypeMapper.upsert(ctx, type);
    }

    public SDataTypeStructure buildDesc(Structure structure) {
        SDataTypeStructure StructureDesc = new SDataTypeStructure();
        StructureDesc.setType(dataTypeMapper.buildDesc(structure));

        DataTypeComponent[] components = structure.getDefinedComponents();
        StructureDesc.setFields(new ArrayList<>());
        for(DataTypeComponent component : components) {
            SDataTypeStructureField field = new SDataTypeStructureField();
            field.setOffset(component.getOffset());
            field.setName(component.getFieldName());
            if(component.getComment() != null)
                field.setComment(component.getComment());
            else field.setComment("");
            field.setType(dataTypeMapper.buildTypeUnitDesc(component.getDataType()));
            StructureDesc.addToFields(field);
        }
        return StructureDesc;
    }

    public void changeStructureByDesc(Structure structure, SDataTypeStructure structDesc) {
        dataTypeMapper.changeTypeByDesc(structure, structDesc.getType());

        structure.deleteAll();
        for(SDataTypeStructureField field : structDesc.getFields()) {
            DataType type = dataTypeMapper.getTypeByDesc(field.getType());
            structure.insertAtOffset(field.getOffset(), type, type.getLength(), field.getName(), field.getComment());
        }

        int byteCountToEnd = structDesc.getType().getSize() - structure.getLength();
        if(byteCountToEnd > 0) {
            ArrayDataType arrType = new ArrayDataType(new Undefined1DataType(dataTypeMapper.dataTypeManager), byteCountToEnd, 1, dataTypeMapper.dataTypeManager);
            structure.add(arrType, byteCountToEnd, "reserved", "required to save the size of the structure");
        }
    }
}
