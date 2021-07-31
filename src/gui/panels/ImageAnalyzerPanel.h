#pragma once
#include "ImageDecorator.h"
#include "decompiler/DecWarningContainer.h"
#include "decompiler/PCode/DecRegisterFactory.h"
#include "decompiler/PCode/Decoders/DecPCodeDecoderX86.h"
#include "decompiler/PCode/ImageAnalyzer/DecImageAnalyzer.h"
#include "imgui_wrapper/Window.h"
#include "imgui_wrapper/controls/AbstractPanel.h"
#include "imgui_wrapper/controls/Button.h"

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

			WarningContainer warningContainer;
			RegisterFactoryX86 registerFactoryX86;
			DecoderX86 decoder(&registerFactoryX86, m_imageDec->getInstrPool(), &warningContainer);

			const ImageAnalyzer imageAnalyzer(m_imageDec->getImage(), m_imageDec->getPCodeGraph(), &decoder, &registerFactoryX86);
			imageAnalyzer.start(m_startOffset);
		}
	};
};