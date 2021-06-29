package sda.sync.mappers;

import ghidra.program.model.data.*;
import ghidra.program.model.data.Enum;
import ghidra.util.InvalidNameException;
import ghidra.util.exception.DuplicateNameException;
import ghidra.util.task.TaskMonitorAdapter;
import sda.Sda;
import sda.ghidra.datatype.*;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.ghidra.shared.STypeUnit;
import sda.sync.IBaseMapper;
import sda.sync.SyncContext;
import sda.util.ObjectHash;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class DataTypeMapper implements IBaseMapper {

    private Sda sda;
    public DataTypeManager dataTypeManager;
    public TypedefTypeMapper typedefTypeMapper;
    public EnumTypeMapper enumTypeMapper;
    public StructureTypeMapper structureTypeMapper;
    public ClassTypeMapper classTypeMapper;
    public SignatureTypeMapper signatureTypeMapper;

    public DataTypeMapper(Sda sda, DataTypeManager dataTypeManager) {
        this.sda = sda;
        this.dataTypeManager = dataTypeManager;
        typedefTypeMapper = new TypedefTypeMapper(sda, this);
        enumTypeMapper = new EnumTypeMapper(sda, this);
        structureTypeMapper = new StructureTypeMapper(sda, this);
        classTypeMapper = new ClassTypeMapper(sda, structureTypeMapper);
        signatureTypeMapper = new SignatureTypeMapper(sda, this);
    }


    @Override
    public void loadToRemove(SDataFullSyncPacket dataPacket) {
        for(Long id : dataPacket.removed_datatypes) {
            DataType dataType = findDataTypeByGhidraId(id);
            if(dataType != null) {
                dataTypeManager.remove(dataType, new TaskMonitorAdapter(true));
            }
        }
    }

    @Override
    public void loadToCreate(SDataFullSyncPacket dataPacket) {
        for(SDataTypeTypedef typedef : dataPacket.typedefs) {
            createTypeByDescIfNotExist(typedef.type);
        }

        for(SDataTypeEnum Enum : dataPacket.enums) {
            createTypeByDescIfNotExist(Enum.type);
        }

        for(SDataTypeStructure struct : dataPacket.structures) {
            createTypeByDescIfNotExist(struct.type);
        }

        for(SDataTypeClass Class : dataPacket.classes) {
            createTypeByDescIfNotExist(Class.structType.type);
        }

        for(SDataTypeSignature signature : dataPacket.signatures) {
            createTypeByDescIfNotExist(signature.type);
        }
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        typedefTypeMapper.load(dataPacket);
        enumTypeMapper.load(dataPacket);
        structureTypeMapper.load(dataPacket);
        classTypeMapper.load(dataPacket);
        signatureTypeMapper.load(dataPacket);
    }

    public void upsert(SyncContext ctx, DataType type) {
    }

    public SDataType buildDesc(DataType dataType) {
        SDataType typeDesc = new SDataType();

        typeDesc.setId(getGhidraId(dataType));
        typeDesc.setName(dataType.getName());

        if(dataType instanceof Structure) {
            typeDesc.setGroup(DataTypeGroup.Structure);
        } else if(dataType instanceof Enum) {
            typeDesc.setGroup(DataTypeGroup.Enum);
        } else if(dataType instanceof TypeDef) {
            typeDesc.setGroup(DataTypeGroup.Typedef);
        } else if(dataType instanceof FunctionDefinition) {
            typeDesc.setGroup(DataTypeGroup.Signature);
        }

        typeDesc.setComment(dataType.getDescription());
        typeDesc.setSize(dataType.getLength());
        return typeDesc;
    }

    public STypeUnit buildTypeUnitDesc(DataType dataType) {
        STypeUnit typeUnitDesc = new STypeUnit();
        typeUnitDesc.setTypeId(getGhidraId(dataType));
        typeUnitDesc.setPointerLvls(new ArrayList<Short>());
        for(short lvl : getTypePointerLvls(dataType)) {
            typeUnitDesc.getPointerLvls().add(lvl);
        }
        return typeUnitDesc;
    }

    public DataType getTypeByDesc(STypeUnit desc) {
        DataType type = findDataTypeByGhidraId(desc.getTypeId());
        if(type == null) {
            return new ByteDataType();
        }

        for(short lvl : desc.getPointerLvls()) {
            if(lvl == 1) {
                type = new PointerDataType(type, dataTypeManager);
            } else {
                type = new ArrayDataType(type, lvl, type.getLength(), dataTypeManager);
            }
        }
        return type;
    }

    public void changeTypeByDesc(DataType dataType, SDataType typeDesc) {
        try {
            dataType.setName(typeDesc.getName());
        } catch (InvalidNameException e) {
            e.printStackTrace();
        } catch (DuplicateNameException e) {
            e.printStackTrace();
        }

        try {
            dataType.setDescription(typeDesc.getComment());
        } catch (UnsupportedOperationException e) {
            e.printStackTrace();
        }
    }

    public DataType findDataTypeByGhidraId(long id) {
        Iterator<DataType> dataTypes = sda.getProgram().getDataTypeManager().getAllDataTypes();
        while(dataTypes.hasNext()) {
            DataType dataType = dataTypes.next();
            if(dataType instanceof Pointer || dataType instanceof Array)
                continue;
            if (getGhidraId(dataType) == id) {
                return dataType;
            }
        }
        return null;
    }

    private void createTypeByDescIfNotExist(SDataType typeDesc) {
        DataType type = findDataTypeByGhidraId(typeDesc.getId());
        if(type == null) {
            dataTypeManager.addDataType(createTypeByDesc(typeDesc), DataTypeConflictHandler.REPLACE_HANDLER);
        }
    }

    private DataType createTypeByDesc(SDataType typeDesc) {
        DataType type = null;
        Category cat = dataTypeManager.getRootCategory().getCategory(Sda.dataTypeCategory);
        if(cat == null) {
            try {
                cat = dataTypeManager.getRootCategory().createCategory(Sda.dataTypeCategory);
            } catch (InvalidNameException e) {
                e.printStackTrace();
            }
        }

        CategoryPath catPath = cat.getCategoryPath();
        switch(typeDesc.getGroup())
        {
            case Typedef:
                type = new TypedefDataType(catPath, typeDesc.getName(), new ByteDataType(), dataTypeManager);
                break;
            case Enum:
                type = new EnumDataType(catPath, typeDesc.getName(), typeDesc.getSize(), dataTypeManager);
                break;
            case Structure:
            case Class:
                type = new StructureDataType(catPath, typeDesc.getName(), typeDesc.getSize(), dataTypeManager);
                break;
            case Signature:
                type = new FunctionDefinitionDataType(catPath, typeDesc.getName(), dataTypeManager);
                break;
        }

        try {
            type.setDescription(typeDesc.getComment());
        } catch (UnsupportedOperationException e) {
            e.printStackTrace();
        }
        return type;
    }

    private static List<Short> getTypePointerLvls(DataType dataType) {
        List<Short> ptr_levels = new ArrayList<Short>();
        if(dataType instanceof Pointer) {
            ptr_levels = getTypePointerLvls(((Pointer) dataType).getDataType());
            ptr_levels.add((short)1);
        } else if(dataType instanceof Array) {
            ptr_levels = getTypePointerLvls(((Array) dataType).getDataType());
            ptr_levels.add((short)((Array)dataType).getNumElements());
        }
        return ptr_levels;
    }

    private static String getTypeName(DataType dataType) {
        if(dataType == null) {
            return "byte";
        }
        if(dataType instanceof Array) {
            return getTypeName(((Array)dataType).getDataType());
        }
        if(dataType instanceof Pointer) {
            return getTypeName(((Pointer)dataType).getDataType());
        }
        return dataType.getName();
    }

    private static long getGhidraId(DataType dataType) {
        ObjectHash hash = new ObjectHash();
        hash.addValue(getTypeName(dataType));
        return hash.getHash();
    }
}
