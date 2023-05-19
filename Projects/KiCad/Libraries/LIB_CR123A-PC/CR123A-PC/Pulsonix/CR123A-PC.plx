PULSONIX_LIBRARY_ASCII "SamacSys ECAD Model"
//1153200/918720/2.50/4/3/Undefined or Miscellaneous

(asciiHeader
	(fileUnits MM)
)
(library Library_1
	(padStyleDef "c480_h320"
		(holeDiam 3.2)
		(padShape (layerNumRef 1) (padShapeType Ellipse)  (shapeWidth 4.800) (shapeHeight 4.800))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 4.800) (shapeHeight 4.800))
	)
	(padStyleDef "s140_h90"
		(holeDiam 0.9)
		(padShape (layerNumRef 1) (padShapeType Rect)  (shapeWidth 1.400) (shapeHeight 1.400))
		(padShape (layerNumRef 16) (padShapeType Rect)  (shapeWidth 1.400) (shapeHeight 1.400))
	)
	(padStyleDef "c140_h90"
		(holeDiam 0.9)
		(padShape (layerNumRef 1) (padShapeType Ellipse)  (shapeWidth 1.400) (shapeHeight 1.400))
		(padShape (layerNumRef 16) (padShapeType Ellipse)  (shapeWidth 1.400) (shapeHeight 1.400))
	)
	(textStyleDef "Normal"
		(font
			(fontType Stroke)
			(fontFace "Helvetica")
			(fontHeight 1.27)
			(strokeWidth 0.127)
		)
	)
	(patternDef "CR123APC" (originalName "CR123APC")
		(multiLayer
			(pad (padNum 1) (padStyleRef s140_h90) (pt 0.000, 0.000) (rotation 90))
			(pad (padNum 2) (padStyleRef c140_h90) (pt 40.600, 0.000) (rotation 90))
			(pad (padNum 3) (padStyleRef c480_h320) (pt 7.600, 0.000) (rotation 90))
			(pad (padNum 4) (padStyleRef c480_h320) (pt 33.000, 0.000) (rotation 90))
		)
		(layerContents (layerNumRef 18)
			(attr "RefDes" "RefDes" (pt 20.300, 0.000) (textStyleRef "Normal") (isVisible True))
		)
		(layerContents (layerNumRef 28)
			(line (pt -1.65 -10.25) (pt 42.25 -10.25) (width 0.025))
		)
		(layerContents (layerNumRef 28)
			(line (pt 42.25 -10.25) (pt 42.25 10.25) (width 0.025))
		)
		(layerContents (layerNumRef 28)
			(line (pt 42.25 10.25) (pt -1.65 10.25) (width 0.025))
		)
		(layerContents (layerNumRef 28)
			(line (pt -1.65 10.25) (pt -1.65 -10.25) (width 0.025))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.65 -10.16) (pt 42.25 -10.16) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt 42.25 -10.16) (pt 42.25 10.25) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt 42.25 10.25) (pt -1.65 10.25) (width 0.2))
		)
		(layerContents (layerNumRef 18)
			(line (pt -1.65 10.25) (pt -1.65 -10.16) (width 0.2))
		)
		(layerContents (layerNumRef Courtyard_Top)
			(line (pt -2.65 11.25) (pt 43.25 11.25) (width 0.1))
		)
		(layerContents (layerNumRef Courtyard_Top)
			(line (pt 43.25 11.25) (pt 43.25 -11.25) (width 0.1))
		)
		(layerContents (layerNumRef Courtyard_Top)
			(line (pt 43.25 -11.25) (pt -2.65 -11.25) (width 0.1))
		)
		(layerContents (layerNumRef Courtyard_Top)
			(line (pt -2.65 -11.25) (pt -2.65 11.25) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(line (pt -2.3 0) (pt -2.3 0) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -2.25, 0) (radius 0.05) (startAngle 180.0) (sweepAngle 180.0) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(line (pt -2.2 0) (pt -2.2 0) (width 0.1))
		)
		(layerContents (layerNumRef 18)
			(arc (pt -2.25, 0) (radius 0.05) (startAngle .0) (sweepAngle 180.0) (width 0.1))
		)
	)
	(symbolDef "CR123A-PC" (originalName "CR123A-PC")

		(pin (pinNum 1) (pt 0 mils 0 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -25 mils) (rotation 0]) (justify "Left") (textStyleRef "Normal"))
		))
		(pin (pinNum 2) (pt 0 mils -300 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -325 mils) (rotation 0]) (justify "Left") (textStyleRef "Normal"))
		))
		(pin (pinNum 3) (pt 0 mils -100 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -125 mils) (rotation 0]) (justify "Left") (textStyleRef "Normal"))
		))
		(pin (pinNum 4) (pt 0 mils -200 mils) (rotation 0) (pinLength 200 mils) (pinDisplay (dispPinName true)) (pinName (text (pt 230 mils -225 mils) (rotation 0]) (justify "Left") (textStyleRef "Normal"))
		))
		(line (pt 200 mils 100 mils) (pt 700 mils 100 mils) (width 6 mils))
		(line (pt 700 mils 100 mils) (pt 700 mils -400 mils) (width 6 mils))
		(line (pt 700 mils -400 mils) (pt 200 mils -400 mils) (width 6 mils))
		(line (pt 200 mils -400 mils) (pt 200 mils 100 mils) (width 6 mils))
		(attr "RefDes" "RefDes" (pt 750 mils 300 mils) (justify Left) (isVisible True) (textStyleRef "Normal"))
		(attr "Type" "Type" (pt 750 mils 200 mils) (justify Left) (isVisible True) (textStyleRef "Normal"))

	)
	(compDef "CR123A-PC" (originalName "CR123A-PC") (compHeader (numPins 4) (numParts 1) (refDesPrefix U)
		)
		(compPin "1" (pinName "-") (partNum 1) (symPinNum 1) (gateEq 0) (pinEq 0) (pinType Unknown))
		(compPin "2" (pinName "+") (partNum 1) (symPinNum 2) (gateEq 0) (pinEq 0) (pinType Unknown))
		(compPin "MH1" (pinName "MH1") (partNum 1) (symPinNum 3) (gateEq 0) (pinEq 0) (pinType Unknown))
		(compPin "MH2" (pinName "MH2") (partNum 1) (symPinNum 4) (gateEq 0) (pinEq 0) (pinType Unknown))
		(attachedSymbol (partNum 1) (altType Normal) (symbolName "CR123A-PC"))
		(attachedPattern (patternNum 1) (patternName "CR123APC")
			(numPads 4)
			(padPinMap
				(padNum 1) (compPinRef "1")
				(padNum 2) (compPinRef "2")
				(padNum 3) (compPinRef "MH1")
				(padNum 4) (compPinRef "MH2")
			)
		)
		(attr "Manufacturer_Name" "TAKACHI")
		(attr "Manufacturer_Part_Number" "CR123A-PC")
		(attr "Mouser Part Number" "")
		(attr "Mouser Price/Stock" "")
		(attr "Arrow Part Number" "")
		(attr "Arrow Price/Stock" "")
		(attr "Description" "LITHIUM BATTERY HOLDER")
		(attr "<Hyperlink>" "https://in.misumi-ec.com/vona2/detail/222000224510/?HissuCode=CR123A-PC")
		(attr "<Component Height>" "16.5")
		(attr "<STEP Filename>" "CR123A-PC.stp")
		(attr "<STEP Offsets>" "X=0;Y=0;Z=0")
		(attr "<STEP Rotation>" "X=0;Y=0;Z=0")
	)

)
