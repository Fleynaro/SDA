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
	public:
		ImageAnalyzerPanel(CE::ImageDecorator* imageDec)
			: AbstractPanel("Image Analyzer Master"), m_imageDec(imageDec)
		{}

	private:
		void renderPanel() override {
			Text::Text(std::string("Image: ") + m_imageDec->getName()).show();
			
			if (Button::StdButton("Start").present()) {
				
			}
		}

		void startAnalysis() {
			using namespace CE;
			using namespace Decompiler;

			WarningContainer warningContainer;
			RegisterFactoryX86 registerFactoryX86;
			DecoderX86 decoder(&registerFactoryX86, m_imageDec->getInstrPool(), &warningContainer);

			const ImageAnalyzer imageAnalyzer(m_imageDec->getImage(), m_imageDec->getPCodeGraph(), &decoder, &registerFactoryX86);
			imageAnalyzer.start(m_imageDec->getImage()->getOffsetOfEntryPoint());
		}
	};
};