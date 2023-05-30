# Readme

## 1 Chinese Version

1. 在 Visual Studio 2022 中运行程序后，输出结果保存在 `/SimpleCalculator`目录下的`output.txt`中.
2. 在`/SimpleCalculator`目录下打开`cmd`，键入命令`type output.txt | bc.exe > validate.txt`即可将测试结果保存至`validate.txt`中. (请注意，本仓库中没有包含 `bc.exe`，因此需要您自己下载。另外，`bc.exe`的依赖库`readline5.dll`也需要您自行下载，并且与`bc.exe`置于同一目录下。这些文件可以在 https://gnuwin32.sourceforge.net/packages/bc.htm 处下载.)
3. 打开`validate.txt`，查找`0`，如果找不到就证明实验程序正确.

## 2 English Version

1. After running the program in Visual Studio 2022, the output is saved in `output.txt` in the `/SimpleCalculator` directory.
2. Open `cmd` in the `/SimpleCalculator` directory, type the command `type output.txt | bc.exe > validate.txt` to save the test results to `validate.txt`. (Note that `bc.exe` is not included in this repository, so you need to download it yourself. In addition, the dependent library `readline5.dll` of `bc.exe` also needs to be downloaded by yourself, and placed in the same directory as `bc.exe`. These files can be downloaded at https://gnuwin32.sourceforge.net/packages/bc.htm)
3. Open `validate.txt`, look for `0`, if you can't find it, it proves that the experimental program is correct.