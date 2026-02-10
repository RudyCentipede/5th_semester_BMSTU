	gs -sDEVICE=pdfwrite \
         -dCompatibilityLevel=1.4 \
         -dNOPAUSE \
         -dOptimize=true \
         -dQUIET \
         -dBATCH \
         -dRemoveUnusedFonts=true \
         -dRemoveUnusedImages=true \
         -dOptimizeResources=true \
         -dDetectDuplicateImages \
         -dCompressFonts=true \
         -dEmbedAllFonts=true \
         -dSubsetFonts=true \
         -dPreserveAnnots=true \
         -dPreserveMarkedContent=true \
         -dPreserveOverprintSettings=true \
         -dPreserveHalftoneInfo=true \
         -dPreserveOPIComments=true \
         -dPreserveDeviceN=true \
         -dMaxInlineImageSize=0 \
		 -sOutputFile="./report/report_compressed.pdf" "./report/report.pdf"

	mv ./report/report_compressed.pdf ./report/report.pdf