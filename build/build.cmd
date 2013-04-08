cd C:\Users\chgu\Dropbox\Master\管线改移系统\build\vc10

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"

msbuild AsdkAcUiSample.vcxproj /Property:Configuration=Debug;Platform=x64

git@github.com:guchanghai/LineReDesignManagement.git