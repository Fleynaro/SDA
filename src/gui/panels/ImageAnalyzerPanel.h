#pragma once
#include "ImageDecorator.h"
#include "decompiler/DecWarningContainer.h"
#include "decompiler/PCode/DecRegisterFactory.h"
#include "decompiler/PCode/Decoders/DecPCodeDecoderX86.h"
#include "decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Button.h"
#include "managers/FunctionManager.h"
#include "managers/ImageManager.h"
#include "managers/TypeManager.h"

namespace GUI
{
	class ImageAnalyzerPanel : public AbstractPanel
	{
		CE::ImageDecorator* m_imageDec;
		CE::Offset m_startOffset;
	public:
		ImageAnalyzerPanel(CE::ImageDecorator* imageDec, CE::Offset startOffset)
			: AbstractPanel("Image Analyzer Master"), m_imageDec(imageDec), m_startOffset(startOffset)
		{}

		ImageAnalyzerPanel(CE::ImageDecorator* imageDec)
			: ImageAnalyzerPanel(imageDec, imageDec->getImage()->getOffsetOfEntryPoint())
		{}

	private:
		void renderPanel() override {
			Text::Text(std::string("Image: ") + m_imageDec->getName()).show();

			using namespace Helper::String;
			Text::Text("Offset: 0x" + NumberToHex(m_startOffset)).show();

			NewLine();
			if (Button::StdButton("Start").present()) {
				startAnalysis();
			}
		}

		void startAnalysis() const {
			using namespace CE;
			using namespace Decompiler;

			const auto project = m_imageDec->getImageManager()->getProject();
			const auto imagePCodeGraph = new ImagePCodeGraph();
			const auto instrPool = new InstructionPool();

			WarningContainer warningContainer;
			RegisterFactoryX86 registerFactoryX86;
			DecoderX86 decoder(&registerFactoryX86, instrPool, &warningContainer);

			PCodeGraphReferenceSearch graphReferenceSearch(project, &registerFactoryX86, m_imageDec->getImage());
			const ImageAnalyzer imageAnalyzer(m_imageDec->getImage(), imagePCodeGraph, &decoder, &registerFactoryX86, &graphReferenceSearch);
			imageAnalyzer.start(m_startOffset);

			m_imageDec->setInstrPool(instrPool);
			m_imageDec->setPCodeGraph(imagePCodeGraph);

			const auto defSignature = project->getTypeManager()->getDefaultFuncSignature();
			for(const auto funcGraph : imagePCodeGraph->getFunctionGraphList()) {
				const auto funcOffset = funcGraph.getStartBlock()->getMinOffset().getByteOffset();
				project->getFunctionManager()->getFactory().createFunction(funcOffset, defSignature, m_imageDec, "func_" + Helper::String::NumberToHex(funcOffset));
			}
		}
	};
};