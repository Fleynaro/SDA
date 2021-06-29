package sda.sync;

import ghidra.program.model.data.DataType;
import ghidra.program.model.data.FunctionDefinition;
import ghidra.program.model.data.Structure;
import ghidra.program.model.data.TypeDef;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeClass;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.mappers.DataTypeMapper;
import sda.sync.mappers.FunctionMapper;
import sda.sync.mappers.GlobalVarMapper;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Sync {

    private Sda sda;
    private DataTypeMapper dataTypeMapper;
    private FunctionMapper functionMapper;
    private GlobalVarMapper globalVarMapper;
    private List<IBaseMapper> mappers = new ArrayList<>();

    public Sync(Sda sda) {
        this.sda = sda;
        dataTypeMapper = new DataTypeMapper(sda, sda.getProgram().getDataTypeManager());
        functionMapper = new FunctionMapper(sda, sda.getProgram().getFunctionManager(), dataTypeMapper);
        globalVarMapper = new GlobalVarMapper(sda, sda.getProgram().getListing(), dataTypeMapper);
        mappers.add(dataTypeMapper);
        mappers.add(functionMapper);
        mappers.add(globalVarMapper);
    }

    public void loadDataFullSyncPacket(SDataFullSyncPacket dataPacket) {
        int transactionId = sda.getProgram().startTransaction("SDA transaction");
        for(IBaseMapper mapper : mappers) {
            mapper.loadToRemove(dataPacket);
            mapper.loadToCreate(dataPacket);
            mapper.load(dataPacket);
        }
        sda.getProgram().endTransaction(transactionId, true);
    }

    public SDataFullSyncPacket buildDataFullSyncPacket() {
        SyncContext ctx = new SyncContext();
        ctx.dataPacket = new SDataFullSyncPacket();
        initDataFullSyncPacket(ctx.dataPacket);
        fillDataFullSyncPacketWithDataTypes(ctx);
        fillDataFullSyncPacketWithFunctions(ctx);
        fillDataFullSyncPacketWithGlobalVars(ctx);
        return ctx.dataPacket;
    }

    private void initDataFullSyncPacket(SDataFullSyncPacket dataPacket) {
        dataPacket.setTypedefs(new ArrayList<>());
        dataPacket.setEnums(new ArrayList<>());
        dataPacket.setStructures(new ArrayList<>());
        dataPacket.setClasses(new ArrayList<>());
        dataPacket.setSignatures(new ArrayList<>());
        dataPacket.setFunctions(new ArrayList<>());
        dataPacket.setGlobalVars(new ArrayList<>());
    }

    private void fillDataFullSyncPacketWithDataTypes(SyncContext ctx) {
        Iterator<DataType> dataTypes = sda.getProgram().getDataTypeManager().getAllDataTypes();
        while(dataTypes.hasNext()) {
            DataType dataType = dataTypes.next();
            if(dataType instanceof TypeDef) {
                dataTypeMapper.typedefTypeMapper.upsert(ctx, (TypeDef)dataType);
            } else if(dataType instanceof Enum) {
                dataTypeMapper.enumTypeMapper.upsert(ctx, (ghidra.program.model.data.Enum)dataType);
            } else if(dataType instanceof Structure) {
                dataTypeMapper.structureTypeMapper.upsert(ctx, (Structure)dataType);
            } else if (dataType instanceof FunctionDefinition) {
                dataTypeMapper.signatureTypeMapper.upsert(ctx, (FunctionDefinition)dataType);
            }
        }
    }

    private void fillDataFullSyncPacketWithFunctions(SyncContext ctx) {
        Iterator<Function> functions = sda.getProgram().getFunctionManager().getFunctions(true);
        int idx = 1000;
        while(functions.hasNext()) {
            if(--idx == 0)
                break;
            Function function = functions.next();
            functionMapper.upsert(ctx, function);
        }
    }

    private void fillDataFullSyncPacketWithGlobalVars(SyncContext ctx) {
        Iterator<Data> it = sda.getProgram().getListing().getDefinedData(true);
        while(it.hasNext()) {
            Data data = it.next();
            if(data.getReferenceIteratorTo().hasNext()) {
                globalVarMapper.upsert(ctx, data);
            }
        }
    }
}
