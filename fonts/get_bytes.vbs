SET FSO = CreateObject("Scripting.FileSystemObject") 

FilePath = "X:\Downloads\FontC.xbm"



ModifiedTime = ""

DO WHILE 1
	SET File = FSO.GetFile(FilePath)
	IF ModifiedTime <> File.DateLastModified THEN
		ModifiedTime = File.DateLastModified
		get_text()
		MSGBOX ModifiedTime
	END IF
	WScript.Sleep 250
LOOP

SUB get_text()
	SET File = FSO.OpenTextFile(FilePath, 1)
	XBMData = ""
	N = 1
	DO UNTIL File.AtEndOfStream
		Line = File.ReadLine
		IF N > 3 THEN
			Line = REPLACE(Line, "};", "")
			Line = TRIM(LINE)
			IF N = 4 THEN
				XBMData = LINE
			ELSE
				XBMData = XBMData & VbCrLf & VbTab & VbTab & LINE
			END IF
		END IF
		N = N + 1
	LOOP
	File.Close

	XBMData = XBMData & ","

	SET Word = CreateObject("Word.Application")
	Word.Visible = True
	SET Doc = Word.Documents.Add()
	SET Selection = Word.Selection
	Selection.TypeText XBMData
	Selection.WholeStory
	Selection.copy
	Doc.close wdDoNotSaveChanges
	Word.quit
END SUB



