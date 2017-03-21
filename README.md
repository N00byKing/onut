# onut
## Oak Nut engine.
2D Game engine focused on rapid development. Aimed at Game Jams.

List of jam projects using it:

* Ottawa Game Jam 2015 - [Cannon Fodder Commander ](https://www.youtube.com/watch?v=Jac9r32uIv0)
* Global Game Jam 2016 - [Fire Whisperers ](https://www.youtube.com/watch?v=SWgFVMk5f2Q)
* Ottawa Game Jam 2016 - [Spy Satellite Showdown ](https://www.youtube.com/watch?v=NHyGlVm2ICA)
* Linux Game Jam 2017 - [Retro Game War ](https://www.youtube.com/watch?v=iCbK8YiOAUQ)

## Windows
### Prerequisites
* Windows 10
* Visual Studio 2013 for windows Desktop (Express or Community)

### C++ game

#### Recommended folder structure
Before you start. Please take note of the following folder structure. It is highly recommended for minimal setup time.
The default asset search paths are configured for this structure. _But it easy to add more using `oContentManager->addSearchPath`._

* `YourGame\`
  * `assets\` Put game assets in here
    * ...
  * `project\`
    * `win\` Windows configuration
      * `YourGame.sln` Your game solution
      * `YourGame.vcxproj` Your game project
      * `YourGame.vcxproj.filters`
  * `src\` Your game source code (.h and .cpp)

#### Setup project
Make sure to follow the recommended folder structure when doing those steps.
1. Clone onut somewhere on your PC.
2. Open Visual Studio and create a new empty win32 solution and project for your game.
2. Add onut project to your solution.
  * Right click your solution -> Add -> Existing project -> `[onut path]\project\win\onut.vcxproj`
4. Add build dependancies for your game to include `onut`.
  * Right click YourGame project -> Build Dependencies -> Project Dependencies -> Check mark `onut`
5. Add references to `onut` from your game.
  * Right click YourGame project -> Add -> References -> Add New Reference -> Check mark `onut`
6. Add include path to your game source and onut includes
  * Right click YourGame project -> Properties -> Configuration Properties -> C/C++ -> General -> Additional Include Directories. Add the following
    * `../../src`
    * `[onut path]/include`
7. Use static runtime libraries. This will make sharing your game .exe easier, and onut is built like that by default.
  * Right click YourGame project -> Properties
  * Top left, in the Configuration dropdown, Select `Debug`.
  * Configuration Properties -> C/C++ -> Code Generation -> Runtime Library. Choose `Multi-threaded Debug (/MTd)`
  * Top left, in the Configuration dropdown, Select `Release`.
  * Configuration Properties -> C/C++ -> Code Generation -> Runtime Library. Choose `Multi-threaded (/MT)`

#### main.cpp
Make sure to define those 5 functions. Otherwise you will get unresolved errors.
```cpp
void initSettings()
{
}

void init()
{
}

void update()
{
}

void render()
{
}

void postRender()
{
}
```
Look at samples to see what can be done here.

### JavaScript Game
#### Compile the executable
1. Clone onut somewhere on your PC.
2. Open the solution `[onut path]/JSStandAlone/JSStandAlone.sln`
3. Build in release.

#### Setting up your JavaScript project
Recommended to use Visual Studio Code.
1. Create a folder for your game somehere on your PC
2. Copy `[onut path]/jsconfig.json`, `[onut path]/typings/onut.d.ts` to YourGame path.
3. Create a `settings.txt` file. Refer to samples to see what can be put in there
4. Copy the JSStandAlone.exe to YourGame path.
5. Create assets and javascript files
6. `main.js` will always be the last JavaScript file executed, use it to initialize stuff.

#### main.js
```javascript
function update(dt) {
    // Update your game here
}

function render() {
}
```

## Raspberry PI 2 B
Use cmake to compile the engine and JSStandAlone.

## Samples
See the `onut/samples/Samples.sln` and `onut/samplesJS/` folders to learn how to use onut.

Enjoy!
