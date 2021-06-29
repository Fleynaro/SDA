package sda.sync.mappers;

import ghidra.program.model.data.DataType;
import ghidra.program.model.data.FunctionDefinition;
import ghidra.program.model.data.ParameterDefinition;
import ghidra.program.model.data.ParameterDefinitionImpl;
import ghidra.program.model.listing.FunctionSignature;
import sda.Sda;
import sda.ghidra.datatype.SDataTypeSignature;
import sda.ghidra.datatype.SFunctionArgument;
import sda.ghidra.packet.SDataFullSyncPacket;
import sda.sync.IMapper;
import sda.sync.SyncContext;

import java.lang.reflect.Parameter;
import java.util.ArrayList;

public class SignatureTypeMapper implements IMapper {

    private Sda sda;
    public DataTypeMapper dataTypeMapper;

    public SignatureTypeMapper(Sda sda, DataTypeMapper dataTypeMapper) {
        this.sda = sda;
        this.dataTypeMapper = dataTypeMapper;
    }

    @Override
    public void load(SDataFullSyncPacket dataPacket) {
        for(SDataTypeSignature signatureDesc : dataPacket.getSignatures()) {
            DataType type = dataTypeMapper.findDataTypeByGhidraId(signatureDesc.getType().getId());
            changeSignatureByDesc((FunctionDefinition)type, signatureDesc);
        }
    }

    public void upsert(SyncContext ctx, FunctionDefinition type) {
        ctx.dataPacket.getSignatures().add(buildDesc(type));
        dataTypeMapper.upsert(ctx, type);
    }

    public SDataTypeSignature buildDesc(FunctionSignature signature) {
        SDataTypeSignature signatureDesc = new SDataTypeSignature();
        signatureDesc.setReturnType(dataTypeMapper.buildTypeUnitDesc(signature.getReturnType()));

        signatureDesc.setArguments(new ArrayList<>());
        for(ParameterDefinition param : signature.getArguments()) {
            SFunctionArgument argument = new SFunctionArgument();
            argument.setName(param.getName());
            argument.setType(dataTypeMapper.buildTypeUnitDesc(param.getDataType()));
            signatureDesc.getArguments().add(argument);
        }
        return signatureDesc;
    }

    private SDataTypeSignature buildDesc(FunctionDefinition definition) {
        SDataTypeSignature sig = buildDesc((FunctionSignature)definition);
        sig.setType(dataTypeMapper.buildDesc(definition));
        return sig;
    }

    private void changeSignatureByDesc(FunctionDefinition signature, SDataTypeSignature signatureDesc) {
        dataTypeMapper.changeTypeByDesc(signature, signatureDesc.getType());

        ParameterDefinition[] params = new ParameterDefinition[signatureDesc.getArguments().size()];
        int paramIdx = 0;
        for (SFunctionArgument argDesc : signatureDesc.getArguments()) {
            params[paramIdx] = new ParameterDefinitionImpl(argDesc.getName(), dataTypeMapper.getTypeByDesc(argDesc.getType()), null);
            paramIdx++;
        }
        signature.setArguments(params);
        signature.setReturnType(dataTypeMapper.getTypeByDesc(signatureDesc.getReturnType()));
    }
}
