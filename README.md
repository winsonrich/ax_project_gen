# ax_project_gen
C++ Project Generator

Command Line:
```
ax_gen.exe gen=<Geneartor> ws=<Workspace File> os=<target OS> cpu=<target CPU> [-gen] [-build] [-ide] [-run] [-verbose] 
```
- `arguments can be in random order`

Example:
```
ax_gen.exe gen=vs2015 ws=examples/001/Hello.axworkspace -gen
```

|`Actions:`||
|-----------|------------------------------|
| -gen		| generate target projects     |
| -build	| build projects               |
| -ide		| open IDE                     |
| -run		| run startup project          |
| -verbose  | more detail in console / log |
|||

|`Generators:`|||
|--------------|-------------------------|---|
| gen=vs2015   | Visual Studio 2015      ||
| gen=xcode    | Xcode                   ||
| gen=makefile | GNU/BSD makefile format | `*support file path with space` <br>`(handwritten Makefile might have problem`<br>`during 2nd degree variable evaluation)` |
||||


|`OS options:`||
|------------|------------|
| os=windows | Windows |
| os=macosx  | MacOSX     |
| os=ios     | iOS Device |
| os=linux   | Linux      |
| os=freebsd | FreeBSD    |
|||

<br>

--------


|`CPU Options:`||
|------------|------------|
| cpu=x86    | Intel / AMD 32-bit CPU  |
| cpu=x86_64 | Intel / AMD 64-bit CPU  |
|||

### Output exampe: ( Visual Studio / Xcode / Makefile )
![Visual Studio Solution](doc/ScreenShots/2017-04-03.png)

### Input example *Hello.axproj*:
```
{
	"group": "MyGroup/MyProgram",
	"type": "cpp_exe",
	"dependencies": ["MyLib"],
	"pch_header": "src/precompiledHeader.h",

	//"unite_build": false,
	//"unite_mega_byte_per_file": 1,
	"files" : [
		"src/*.cpp",
		"*.axproj"
	],
	"exclude_files": [
	],		

	"config": {
		"cpp_defines": [],
		"cpp_flags": [],
		"include_dirs": ["src"],
		"link_dirs": [],
		"link_files": [],
		"link_flags": [],
		"compiler==gcc": {
			"link_flags":["-lm"]			
		},
		"compiler==clang": {
			"link_flags":["-lm"]			
		}
	}
}

```