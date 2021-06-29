package sda;

import ghidra.app.decompiler.*;
import ghidra.app.decompiler.component.DecompilerUtils;
import ghidra.app.decompiler.flatapi.FlatDecompilerAPI;
import ghidra.app.plugin.PluginCategoryNames;
import ghidra.app.plugin.ProgramPlugin;
import ghidra.framework.plugintool.PluginInfo;
import ghidra.framework.plugintool.PluginTool;
import ghidra.framework.plugintool.util.PluginStatus;
import ghidra.program.flatapi.FlatProgramAPI;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressFormatException;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;

import ghidra.program.model.pcode.*;
import ghidra.program.model.symbol.*;
import ghidra.program.model.util.CodeUnitInsertionException;
import ghidra.util.task.TaskMonitorAdapter;

import sda.ghidra.Server;
import sda.ghidra.datatype.DataTypeGroup;
import sda.ghidra.datatype.SDataType;
import sda.ghidra.datatype.SDataTypeEnum;
import sda.ghidra.datatype.SDataTypeStructure;
import sda.util.DebugConsole;

import java.util.*;

@PluginInfo(
        status = PluginStatus.UNSTABLE,
        packageName = "SDA plugin",
        category = PluginCategoryNames.MISC,
        shortDescription = "For communicating with SDA",
        description = "This plugin is the server that provides a set of abilities to interact it"
)

public class SdaPlugin extends ProgramPlugin {

    private Sda sda;
    private Server server;

    public SdaPlugin(PluginTool tool) {
        super(tool, true, true);
    }

    @Override
    protected void programOpened(Program program) {
        DebugConsole.info(this, "program " + program.getName() + " opened(ver = 2).");
        sda = new Sda(program);
        server = new Server(sda, 9090);
        server.start();

        //test1(program);

        //datatypes
        {
            //cats
            if(true) {
                Category root = program.getDataTypeManager().getRootCategory();
                Category[] cats = root.getCategories();

                for (Category c : cats) {
                    CategoryPath path = c.getCategoryPath();
                    path.getPath();
                    DataType[] types = c.getDataTypes();
                    for (DataType type : types) {

                    }
                }
            }

            //func decl
            if(true) {
                Iterator<DataType> dataTypes = program.getDataTypeManager().getAllDataTypes();
                while(dataTypes.hasNext()) {
                    DataType dataType = dataTypes.next();
                    if(dataType instanceof FunctionDefinition) {
                        FunctionDefinition def = (FunctionDefinition)dataType;
                        if(def.getName().equals("ScreenVTable_zero")) {
                            DataType retType = def.getReturnType();
                            retType.getAlignment();
                        }
                    }
                }
            }

            //built in types
            if(true) {
                List<DataType> types = new ArrayList<>();
                Iterator<DataType> dataTypes = program.getDataTypeManager().getAllDataTypes();
                while(dataTypes.hasNext()) {
                    DataType dataType = dataTypes.next();
                    if(dataType instanceof BuiltIn) {
                        types.add(dataType);
                    }
                }

                types.size();
            }

            //global vars
            if(true) {
                Iterator<Data> it = program.getListing().getDefinedData(sda.getAddressByOffset(0x236ade0),true);
                if(it.hasNext()) {
                    Data data = it.next();
                    String[] names = data.getNames();
                    String fname = data.getFieldName();
                    String comment = data.getComment(0);

                    Symbol symbol = data.getPrimarySymbol();
                    SourceType src = symbol.getSource();

                    if(symbol.getObject() instanceof Data) {
                        src.getDisplayString();
                    }


                    int transactionId = sda.getProgram().startTransaction("SDA test tr");
                    data.setComment(0, "hi!");
                    sda.getProgram().endTransaction(transactionId, true);

                    /*ReferenceIterator refIt = data.getReferenceIteratorTo();
                    if(refIt.hasNext()) {
                        while(refIt.hasNext()) {
                            Reference ref = refIt.next();
                            Address addr = ref.getFromAddress();
                            int idx = ref.getOperandIndex();
                            RefType refTyp = ref.getReferenceType();
                            SourceType src = ref.getSource();

                            addr.getOffset();
                        }

                        gvars.add(data);
                    }

                    Reference[] refs = data.getReferencesFrom();
                    for(Reference ref : refs) {
                        ref.getFromAddress();
                    }*/
                }

                if(false) {
                    List<Symbol> symbols = new ArrayList<>();
                    Symbol sym;
                    int count = 0;
                    Iterator<Symbol> it2 = program.getSymbolTable().getDefinedSymbols();
                    while (it2.hasNext()) {
                        Symbol symbol = it2.next();
                        count++;

                        if (symbol.getReferenceCount() > 0 && (symbol.getSymbolType() == SymbolType.GLOBAL || symbol.getSymbolType() == SymbolType.GLOBAL_VAR)) {
                            symbols.add(symbol);
                            if (symbol.getName().equals("World")) {
                                sym = symbol;
                            }
                        }
                    }
                    symbols.size();
                }
            }
        }


        //functions
        if(false)
        {
            Address addr = program.getMinAddress();
            try {
                addr = addr.getAddress("0xd02e25");
            } catch (AddressFormatException e) {
                e.printStackTrace();
            }

            Function function = null;
            FunctionIterator functions = program.getFunctionManager().getFunctions(true);
            while (functions.hasNext()) {
                function = functions.next();
                if(function.getName().equals("Screen3D_getVolume")) {
                   break;
                }
            }

            if(function == null)
                return;

            FlatDecompilerAPI api = new FlatDecompilerAPI(new FlatProgramAPI(program));
            String code = "";
            try {
                code = api.decompile(function);
            } catch (Exception e) {
                e.printStackTrace();
            }

            DataType ret_type = function.getReturnType();
            DataType arg_type = function.getSignature().getArguments()[0].getDataType();

            if(ret_type instanceof Pointer) {
                DebugConsole.info(this, "this is a pointer!");
            }

            //tests
            if(true)
            {
                DebugConsole.info(this, "> tags:");
                Set<FunctionTag> set = function.getTags();
                for (FunctionTag tag : set) {
                    DebugConsole.info(this, ">> tag: " + tag.getName());
                }
            }

            if(true)
            {
                DebugConsole.info(this, "> vars:");
                Variable[] vars = function.getAllVariables();
                for (Variable var : vars) {
                    DebugConsole.info(this, ">> variable = " + var.getName());
                }
            }
            if(false)
            {
                DebugConsole.info(this, "> body:");
                AddressRangeIterator ranges = function.getBody().getAddressRanges();
                while (ranges.hasNext()) {
                    AddressRange range = ranges.next();
                    DebugConsole.info(this, ">> range: " + range.getMinAddress().toString() + " to " + range.getMaxAddress().toString());
                }
            }

            if(false)
            {
                DebugConsole.info(this, "> called functions:");
                TaskMonitorAdapter tm = new TaskMonitorAdapter(false);
                Set<Function> calledFunctions = function.getCalledFunctions(tm);
                for (Function it : calledFunctions) {
                    DebugConsole.info(this, ">> function = " + it.getName());
                }
            }

            if(false)
            {
                HighFunction hf = new HighFunction(function, program.getLanguage(), program.getCompilerSpec(), new PcodeDataTypeManager(program), true);
                int numVarnodes = hf.getNumVarnodes();
                Iterator<VarnodeAST> varnodes = hf.getVarnodes(function.getEntryPoint());
                while(varnodes.hasNext()) {
                    VarnodeAST varnode = varnodes.next();
                    String name = varnode.getHigh().getName();
                }
            }

            if(false)
            {
                DecompInterface decompiler = new DecompInterface();
                decompiler.openProgram(program);
                TaskMonitorAdapter tm = new TaskMonitorAdapter(false);

                DecompileResults decompRes =
                        decompiler.decompileFunction(function, 0, tm);

                if(decompRes.decompileCompleted()) {
                    HighFunction hf = decompRes.getHighFunction();

                    ClangTokenGroup group = decompRes.getCCodeMarkup();
                    int numChildren1 = group.numChildren();
                    Address a1 = group.getMinAddress();
                    Address a2 = group.getMaxAddress();
                    String s1 = group.toString();
                    if(numChildren1 > 0) {
                        ClangNode node = group.Child(2);
                        int numChildren2 = node.numChildren();
                        Address a3 = node.getMinAddress();
                        Address a4 = node.getMaxAddress();
                        String s2 = node.toString();
                        DebugConsole.info(this, "s2 = " + s2);

                        ArrayList<ClangLine> lines = DecompilerUtils.toLines(group);
                        if(lines.size() >= 30) {
                            String line1 = lines.get(0).toString();
                            String line2 = lines.get(1).toString();
                            String line3 = lines.get(2).toString();
                            ArrayList<ClangToken> tokens = lines.get(20).getAllTokens();
                            for(ClangToken token : tokens) {
                                String text = token.getText();
                                int stype = token.getSyntaxType();
                            }
                        }
                    }

                    int numParams222 = hf.getLocalSymbolMap().getNumParams();
                    Iterator<HighSymbol> symbols = hf.getLocalSymbolMap().getSymbols();
                    while(symbols.hasNext()) {
                        HighSymbol symbol = symbols.next();
                        String name = symbol.getName();
                        String addr1 = symbol.getHighVariable().getStorage().getMinAddress().toString();
                        String regName = symbol.getHighVariable().getStorage().getRegister().getName();
                        Varnode[] varnodes1 = symbol.getHighVariable().getStorage().getVarnodes();

                        DebugConsole.info(this, ">symbol = " + name + "("+ symbol.getHighVariable().getRepresentative().getAddress().toString() +")");

                        for(Varnode var : varnodes1) {
                            String addr2 = var.getAddress().toString();
                            DebugConsole.info(this, ">>> addr = " + addr2);
                        }
                    }

                    int numVarnodes = hf.getNumVarnodes();
                    DebugConsole.info(this, "numVarnodes = " + numVarnodes);

                    Iterator<VarnodeAST> varnodes = hf.getVarnodes(function.getEntryPoint());
                    while(varnodes.hasNext()) {
                        VarnodeAST varnode = varnodes.next();
                        String name = varnode.getHigh().getName();
                        DebugConsole.info(this, "varname = " + name);
                    }
                }
            }

            DebugConsole.info(this, "function = " + function.getName());
        }
    }

    private void test1(Program program) {
        if(false) {
            int id = program.startTransaction("SDA: change enums");
            int id2 = program.startTransaction("SDA: change enums2");
            Structure tt1 = (Structure) program.getDataTypeManager().getDataType(new DataTypePath("/SDA", "Entity"));
            program.endTransaction(id2, true);
            tt1.add(new ArrayDataType(new ByteDataType(), 222, 1));
            program.endTransaction(id, true);

            /*DataType tt1_ = program.getDataTypeManager().findDataTypeForID(tt1.getUniversalID());
            Long val1 = tt1.getUniversalID().getValue();

            DoubleDataType tt2 = new DoubleDataType(program.getDataTypeManager());
            DataType tt2_ = program.getDataTypeManager().findDataTypeForID(tt1.getUniversalID());

            Pointer ptr = new PointerDataType(tt1, program.getDataTypeManager());
            Long val2 = ptr.getUniversalID().getValue();
            Long val3 = ptr.getDataType().getUniversalID().getValue();

            Pointer ptr2 = new PointerDataType(new PointerDataType(tt1, program.getDataTypeManager()), program.getDataTypeManager());
            Long val4 = ptr2.getUniversalID().getValue();*/
        }

        DebugConsole.info(this, ">>> " + "getFunctionCount() = " + " " + program.getFunctionManager().getFunctionCount());

        /*Integer count = 0;
        Boolean isFirst = true;
        FunctionIterator functions = program.getFunctionManager().getFunctions(true);
        while (functions.hasNext()) {
            Function func = functions.next();

            if(func.getReturnType() instanceof FloatDataType) {
                count++;
            }

            if(isFirst && func.getReturnType() instanceof PointerDataType) {
                PointerDataType dataType = (PointerDataType)func.getReturnType();
                if(dataType.getDataType() instanceof FloatDataType) {
                    isFirst = false;
                    DebugConsole.info(this, "for " + func.getName() + "> dataType.getDisplayName() = " + dataType.getDisplayName() + "("+ dataType.getName() +")");
                }
            }
        }
        DebugConsole.info(this, ">>> " + "count = " + " " + count);*/
    }

    @Override
    protected void programClosed(Program program) {
        server.stop();
        DebugConsole.info(this, "program " + program.getName() + " closed.");
    }

    @Override
    protected void init() {
    }

    @Override
    protected void dispose() {
    }
}
