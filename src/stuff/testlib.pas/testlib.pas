{**
 * WebTester Server - server of on-line testing system
 *
 * Pascal implementation for testlib.
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 *}

unit testlib;

interface

const
  _OK   = $0000;
  _WA   = $0001;
  _PE   = $0002;
  _CR   = $0004; { System error }
  _FAIL = $0008; { Test's fail. User's solution is better than judges' }

const
  READ_ONLY = 0;
  WRITE_ONLY = 1;
  READ_WRITE = 2;

const
  EOFChar = #26;

  EOLNChars = [#10, #13, EOFChar];
  blanks    = [#9, #32, #10, #13];

  NumCharsBefore = [#10, #13, ' '];
  NumCharsAfter  = [#10, #13, ' '];

  MaxStrLen = 65535;

type

  Real=Extended; { !!! Warning !!! }

  TCharset = set of char;

  TMode = (TFM_INPUT, TFM_OUTPUT, TFM_ANSWER);

  TTestlibFile = object
    public
    constructor Create  (fn: string; m: TMode);
    destructor  Destroy;

    function ReadInteger: integer;    { Read integer value from file }
    function ReadLongint: longint;    { Read longint from file }
    function ReadFloat: real;         { Read double from file }
    function ReadReal:  real;         { Read real from file }

    function NextChar : char;     { Read char from file and move to next }
    function CurChar  : char;     { Just return current char }

    function Eof     : boolean; { Check for end of file }
    function Eoln    : boolean; { Check for end of line }
    function SeekEof : boolean; { Seek for end of file }
    function SeekEoln: boolean; { Seek for end on line }

    procedure NextLine;                 { Goto next line }
    procedure Skip (charset: TCharset); { Skip characters from set. }
                                        { Do not generate errors }

    function ReadWord (before, after: TCharset): string;

    function ReadString: string;

    procedure ReSet;

    private
    ch     : char;
    stream : File of Char;
    mode   : TMode;

    function ReadNumber: Real;

    procedure Quit (code: integer; desc: string);
  end;

  procedure Quit (code: integer; desc: string);

var
  inf, ouf, ans: TTestlibFile;
  initialized, silent, colorized: boolean;
  i: integer;
  oldfilemode: byte;

implementation

{$IfDef FPC}
uses crt;
{$EndIf}

{**** TTestlibFile ****}

{ Constructor }
constructor TTestlibFile.Create (fn: string; m: TMode);
begin
  assign (stream, fn);

  oldfilemode := filemode;
  filemode := READ_ONLY;

  {$i-}
  System.reset (stream);
  {$i+}

  filemode := oldfilemode;
  m := mode;

  if (ioResult <> 0) then
  begin
    if m = TFM_OUTPUT
      then Quit (_PE, 'File not found: ' + fn)
      else Quit (_CR, 'File not found: ' + fn);
  end;

  if (System.Eof (stream))
    then ch := EOFChar
    else read (stream, ch);
end;

{ Destructor }
destructor TTestlibFile.Destroy;
begin
  {$i-}
  close (stream);
  {$i+}
end;

{ Reset fiel position }
procedure TTestlibFile.ReSet;
begin
  System.reset (stream);

  if (System.Eof (stream))
    then ch := EOFChar
    else read (stream, ch);
end;

{ Read integer value from file }
function TTestlibFile.ReadInteger: integer;
var a: longint;
begin
  a := trunc (ReadNumber);

  if (a < -32768) or (a > 32767) then
  begin
    Quit (_PE, 'ReadInteger can work ONLY with integer values.');
  end;

  ReadInteger := a;
end;

{ Read longint from file }
function TTestlibFile.ReadLongint: longint;
begin
  ReadLongInt := trunc (ReadNumber);
end;

 { Read double from file }
function TTestlibFile.ReadFloat: real;
begin
  ReadFloat := ReadNumber;
end;

 { Read real from file }
function TTestlibFile.ReadReal: real;
begin
  ReadReal := ReadNumber;
end;

{ Read char from file and move to next }
function TTestlibFile.NextChar: char;
begin
  NextChar := CurChar;

  if System.Eof (stream) then
  begin
    ch := EOFChar;
    exit;
  end;

  {$i-}
  read (stream, ch);
  {$i+}

  if IOResult <> 0 then
  begin
    Quit (_CR, 'Disk read error');
  end;
end;

{ Just return current char }
function TTestlibFile.CurChar: char;
begin
  curchar := ch;
end;

{ Check for end of file }
function TTestlibFile.eof: boolean;
begin
  eof := CurChar = EOFChar;
end;

{ Seek for end of file }
function TTestlibFile.SeekEOF: boolean;
begin
  while ((ord (CurChar) <= 32) or (CurChar in blanks)) and
         (CurChar <> EOFChar) do
  begin
    NextChar;
  end;

  seekeof := CurChar = EOFChar;
end;

{ Check for end of line }
function TTestlibFile.eoln: boolean;
begin
  eoln := CurChar in EOLNChars;
end;

{ Seek for end on line }
function TTestlibFile.SeekEoln: boolean;
begin
  while (CurChar in blanks) and
        not (CurChar in EOLNChars) and not eof do
  begin
    NextChar;
  end;

  seekeoln := CurChar in EOLNChars;
end;

{ Goto next line }
procedure TTestlibFile.NextLine;
begin
  while not (CurChar in EOLNChars) and not eof do
  begin
    NextChar;
  end;

  if CurChar = #13 then NextChar;
  if CurChar = #10 then NextChar;
end;

{ Skip characters from set. Do not generate errors }
procedure TTestlibFile.Skip (charset: TCharset);
begin
  while (CurChar in charset) and not eof do
  begin
    NextChar;
  end;
end;

function TTestlibFile.ReadWord (before, after: TCharset): string;
var
  s: string;
begin
  skip (before);
  s := '';

  while not (CurChar in after) and not eof do
  begin
    if (length (s) >= MaxStrLen - 1) then
    begin
      Quit (_PE, 'Word is too long');
    end;

    s := s + CurChar;
    NextChar;
  end;

  ReadWord := s;
end;

function TTestlibFile.ReadNumber: Real;
var
  s:    string;
  res:  real;
  code: integer;

begin

  if seekeof then
  begin
    Quit (_PE, 'Unexpected end of file');
  end;

  s := ReadWord (NumCharsBefore, NumCharsAfter);

  val (s, res, code);

  if (code <> 0) then
  begin
    Quit (_PE, 'Wrong integer format');
  end;

  ReadNumber := res;
end;

function TTestlibFile.ReadString: string;
var s: string;
begin
  s := '';

  if CurChar = EOFChar then
  begin
    Quit (_PE, 'Unexpected end of file');
  end;

  while not (CurChar in EOLNChars) do
  begin
    if (length (s) > MaxStrLen - 1) then
    begin
      Quit (_PE, 'String to long');
    end;

    s:=s+CurChar;
    NextChar;
  end;

  if CurChar=#13 then NextChar;
  if CurChar=#10 then NextChar;

  ReadString:=s;
end;

procedure TTestlibFile.Quit (code: integer; desc: string);
begin
  if code = _PE then
  begin
    if mode = TFM_INPUT then
    begin
      code := _CR;
      desc := desc + ' in input file';
    end else if mode = TFM_ANSWER then
    begin
      code := _CR;
      desc := desc + ' in answer file';
    end;
  end;

  testlib.Quit (code, desc);
end;

{**** Some internal stuff ****}
procedure Quit (code: integer; desc: string);
begin
  {** Post-checking of output file **}

  { Check for extra information }
  if (initialized) and (code = _OK) and (not ouf.SeekEof) then
  begin
    code := _PE;
    desc := 'Extra information in output file';
  end;

  { Set text color }
{$IfDef FPC}
  if colorized then
  begin
    if code <> _OK
      then TextColor (LightRed)
      else TextColor (LightGreen);
  end;
{$EndIf}

  { Write error message }
  case code of
    _OK: write ('OK');
    _WA: write ('WA');
    _PE: write ('PE');
    _CR: write ('CR');
  end;

{$IfDef FPC}
  if colorized then
  begin
    TextColor (LightGray);
  end;
{$EndIf}

  if desc<>'' then
    writeln ('  ', desc) else
    writeln;

  { For some sound messaging }
{$IfDef FPC}
  if (code <> _OK) and (not silent) then
  begin
    sound (1000);
    delay (200);
    nosound;
  end;
{$EndIf}

  { Terminate executing of checker }
  halt (code);
end;

procedure Usage;
begin
  Quit (_CR, 'Usage: checker <input file> ' +
        '<output file> <answer file> [-s] [-nc]');
end;

begin
  colorized := true;

  if (ParamCount < 3) or (ParamCount > 5) then
  begin
    Usage;
  end;

  for i := 4 to ParamCount do
  begin
    if (ParamStr (i) = '-s')  then silent := true else
    if (ParamStr (i) = '-nc') then colorized := false else
      Usage;
  end;

  inf.Create (ParamStr (1), TFM_INPUT);
  ouf.Create (ParamStr (2), TFM_OUTPUT);
  ans.Create (ParamStr (3), TFM_ANSWER);

  initialized := true;
end.
