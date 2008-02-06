var FSO = WScript.CreateObject("Scripting.FileSystemObject");
var EntriesFile = FSO.OpenTextFile(".svn\\entries", 1, false);

EntriesFile.SkipLine();
EntriesFile.SkipLine();
EntriesFile.SkipLine();

var revision, date_time;

revision = Number(EntriesFile.ReadLine());

EntriesFile.SkipLine();
EntriesFile.SkipLine();
EntriesFile.SkipLine();
EntriesFile.SkipLine();
EntriesFile.SkipLine();

date_time = EntriesFile.ReadLine();

var newData;

newData  = "#ifndef __SVN_REVISION_H__\n";
newData += "#define __SVN_REVISION_H__\n";
newData += " #define SVN_REVISION \"" + revision + "\"\n";
newData += " #define SVN_DATE \"" + date_time.substr(0, 10) + "\"\n";
newData += " #define SVN_TIME \"" + date_time.substr(11, 8) + "\"\n";
newData += "#endif // __SVN_REVISION_H__\n";

EntriesFile.Close();

var oldData = "";

if( FSO.FileExists("..\\src\\shared\\svn_revision.h") )
{
  var HeaderFile = FSO.OpenTextFile("..\\src\\shared\\svn_revision.h", 1, false);
  var oldData = HeaderFile.ReadAll();
  HeaderFile.Close();
}

if(oldData != newData)
{
  var OutputFile = FSO.CreateTextFile("..\\src\\shared\\svn_revision.h", true);
  OutputFile.Write(newData);
  OutputFile.Close();
}


