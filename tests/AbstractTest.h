#pragma once
//SDA
#include <Program.h>
#include <Project.h>
#include <Manager/Managers.h>

//gtest
#define _DEBUG
#undef NDEBUG
#include "gtest/gtest.h"

using namespace CE;

class ProgramFixture : public ::testing::Test
{
    bool m_isClear = false;

protected:
    CE::Program* m_program;
    CE::Project* m_project;

    DB::ITransaction* m_tr;
    CE::TypeManager* m_typeManager;
    CE::SymbolManager* m_symManager;
    CE::SymbolTableManager* m_symTabManger;
    CE::FunctionManager* m_funcManager;
public:
    ProgramFixture()
    {
        m_program = new Program;
    }

    ~ProgramFixture() {
        delete m_program;
        delete m_project;
    }

protected:
    void createProject(const fs::path& dir) {
        auto prj_dir = m_program->getExecutableDirectory() / dir;
        fs::remove_all(prj_dir);
        m_project = m_program->getProjectManager()->createProject(prj_dir);
        initProject();
    }

    void loadProject(const fs::path& dir) {
        auto prj_dir = m_program->getExecutableDirectory() / dir;
        m_project = m_program->getProjectManager()->loadProject(prj_dir);
        initProject();
        m_project->load();
    }

    virtual void initProject() {
        m_project->initDataBase("database.db");
        m_project->initManagers();

        // for being short
        m_tr = m_project->getTransaction();
        m_typeManager = m_project->getTypeManager();
        m_symManager = m_project->getSymbolManager();
        m_symTabManger = m_project->getSymTableManager();
        m_funcManager = m_project->getFunctionManager();
    }

    CE::DataTypePtr findType(std::string typeName, std::string typeLevel = "") {
        return DataType::GetUnit(m_typeManager->findTypeByName(typeName), typeLevel);
    }
};

#include <Decompiler/Decompiler.h>
#include <Decompiler/LinearView/DecLinearView.h>
#include <Decompiler/LinearView/DecLinearViewOptimization.h>
#include <Decompiler/LinearView/DecLinearViewSimpleOutput.h>
#include <Decompiler/SDA/Symbolization/DecGraphSymbolization.h>
#include <Decompiler/SDA/Optimizaton/SdaGraphFinalOptimization.h>
#include <Decompiler/PCode/Decoders/DecPCodeDecoderX86.h>
#include <Decompiler/PCode/DecPCodeConstValueCalc.h>
#include <Decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h>
//#include <Decompiler/Graph/Analyzer/ImagePCodeGraphAnalyzer.h>
#include <Decompiler/DecMisc.h>
#include <Module/Image/SimpleBufferImage.h>
#include <Module/Image/VectorBufferImage.h>

class ProgramDecFixture : public ProgramFixture
{
public:
    ProgramDecFixture()
    {}

protected:
    RegisterFactoryX86 m_registerFactoryX86;
    bool m_isOutput = true;
    CE::DataType::Structure* m_vec3D = nullptr;
    CE::DataType::Structure* m_vecExt3D = nullptr;
    CE::DataType::Structure* m_vec4D = nullptr;
    CE::DataType::Structure* m_matrix4x4 = nullptr;
    FunctionSignature* m_defSignature = nullptr;
    AddressSpace* m_addressSpace = nullptr;
    ImageDecorator* m_testImage = nullptr;

    void createTestDataTypes() {
        m_vec3D = m_typeManager->getFactory().createStructure("testVector3D", "");
        m_vec3D->addField(0x4 * 0, "x", findType("float", ""));
        m_vec3D->addField(0x4 * 1, "y", findType("float", ""));
        m_vec3D->addField(0x4 * 2, "z", findType("float", ""));

        m_vecExt3D = m_typeManager->getFactory().createStructure("testVectorExt3D", "");
        m_vecExt3D->addField(0x8 * 0, "x", findType("float", ""));
        m_vecExt3D->addField(0x8 * 1, "y", findType("float", ""));
        m_vecExt3D->addField(0x8 * 2, "z", findType("float", ""));

        m_vec4D = m_typeManager->getFactory().createStructure("testVector4D", "");
        m_vec4D->addField(0x4 * 0, "x", findType("float", ""));
        m_vec4D->addField(0x4 * 1, "y", findType("float", ""));
        m_vec4D->addField(0x4 * 2, "z", findType("float", ""));
        m_vec4D->addField(0x4 * 3, "w", findType("float", ""));

        m_matrix4x4 = m_typeManager->getFactory().createStructure("testMatrix4x4", "");
        m_matrix4x4->addField(m_vec4D->getSize() * 0, "vec1", GetUnit(m_vec4D));
        m_matrix4x4->addField(m_vec4D->getSize() * 1, "vec2", GetUnit(m_vec4D));
        m_matrix4x4->addField(m_vec4D->getSize() * 2, "vec3", GetUnit(m_vec4D));
        m_matrix4x4->addField(m_vec4D->getSize() * 3, "vec4", GetUnit(m_vec4D));

        m_defSignature = m_typeManager->getFactory().createSignature(DataType::IFunctionSignature::FASTCALL, "defSignature");
        m_defSignature->addParameter("param1", findType("uint32_t"));
        m_defSignature->addParameter("param2", findType("uint32_t"));
        m_defSignature->addParameter("param3", findType("uint32_t"));
        m_defSignature->addParameter("param4", findType("uint32_t"));
        m_defSignature->addParameter("param5", findType("uint32_t"));
        m_defSignature->setReturnType(findType("uint32_t"));
    }

    void initProject() override {
        ProgramFixture::initProject();
        createTestDataTypes();
        m_addressSpace = m_project->getAddrSpaceManager()->createAddressSpace("addressSpace");
        m_testImage = m_project->getImageManager()->createImage(m_addressSpace, ImageDecorator::IMAGE_PE, "testImage");
    }

    void showDecGraph(DecompiledCodeGraph* decGraph, bool minInfo = false) {
        LinearViewSimpleOutput output(Misc::BuildBlockList(decGraph), decGraph);
        if (m_isOutput) {
            if (minInfo)
                output.setMinInfoToShow();
            output.show();
            out("******************\n\n\n");
        }
    }

    // print message
    void out(const char* fmt, ...) {
        if (!m_isOutput)
            return;
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }

    // returns array of bytes on specified address of some function (seeing RET instruction)
    static std::vector<byte> GetFuncBytes(void* addr) {
        auto size = CalculateFuncSize((byte*)addr, 0);
        return std::vector<byte>((byte*)addr, (byte*)addr + size);
    }

    // calculates the function size, seeing RET instruction
    static int CalculateFuncSize(byte* addr, bool endByRet = false) {
        int size = 0;
        while (!(addr[size] == 0xC3 && addr[size + 1] == 0xCC))
            size++;
        return size + 1;
    }
};