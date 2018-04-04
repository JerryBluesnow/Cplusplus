
# This is for making down working tips for visual studio code(vscode)
# configure to support C++ 
https://blog.csdn.net/bat67/article/details/76095813

## Files->Preferences->settings:
    // Place your settings in this file to overwrite the default settings
    {
    //"http.proxy": "135.245.48.34:8000",
    "http.proxy": "http://cnproxy.cn.alcatel-lucent.com/proxy.pac",
    
    "window.zoomLevel": -1,

    "editor.formatOnType": true,
    "editor.formatOnSave": false,

    /* C++ */
    // clang-format is downloaded automatically with C/C++ extension, not clang-format extension
    //"clang-format.executable": "C:\\Users\\jzhan107\\.vscode\\extensions\\ms-vscode.cpptools-0.15.0\\LLVM\\bin\\clang-format.exe",
    // you can got clang-format bianry from http://llvm.org/builds/
    "clang-format.executable": "D:\\software\\clang-format-r325576.exe",
    "clang-format.style": "file",
    "files.exclude": {
        "**/.git": true,
        "**/.svn": true,
        "**/.hg": true,
        "**/CVS": true,
        "**/.DS_Store": true,
        "**/*.exe": true
    },
    
    /* Python */
    "python.pythonPath": "C:\\Python27\\",
    "python.linting.flake8Enabled": true,

    "shellformat.path": "C:\\Users\\jzhan107\\.vscode\\extensions\\foxundermoon.shell-format-1.1.1\\shfmt_v2.0.0_windows_amd64.exe",
        // Shell commands, see default for examples.
        "startanyshell.shells": [
            {
                "description": "Windows Command Prompt",
                "command": "start \"%description%\" /WAIT %comspec%"
            },
            {
                "description": "Git Bash 2",
                "command": "\"C:\\Program Files\\Git\\git-bash.exe\" \"--cd=%path%\""
            },
            {
                "description": "Windows Powershell",
                "command": "start \"%description%\" powershell.exe -noexit"
            },
            {
                "description": "Developer Command Prompt for VS2015",
                "command": "start \"%description%\" /WAIT \"%HOMEDRIVE%\\Program Files (x86)\\Microsoft Visual Studio 14.0\\Common7\\Tools\\VsDevCmd.bat\""
            },
            {
                "description": "Cmder",
                "command": "cmder.exe /start \"%path%\""
            }
        ],
        "explorer.confirmDelete": false,
        "terminal.integrated.shell.windows": "C:\\Program Files\\Git\\bin\\bash.exe"
    }

## Hello