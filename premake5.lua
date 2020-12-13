workspace "pixart_creator"
    architecture "x86_64"
    configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "dependencies/GLFW/include"
IncludeDir["Glad"] = "dependencies/Glad/include"
IncludeDir["Nuklear"] = "dependencies/Nuklear"



group "Dependencies"
include "dependencies/GLFW"
include "dependencies/Glad"


group ""

project "editor"
    kind "ConsoleApp"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    
    files
    {
      "./include/**.h",
      "./source/**.c",

    }

    includedirs
    {
      "./include",
      "%{IncludeDir.GLFW}",
      "%{IncludeDir.Glad}"
    }

    defines
    {
      "_CRT_SECURE_NO_WARNINGS",
	    "GLFW_INCLUDE_NONE", 
    }

    links 
    { 
      "GLFW",
      "Glad",
      "dl",
      "m"
    }


    filter "system:windows"
      system "windows"
      links { "OpenGL32" }
      defines { "SW_PLATFORM_WINDOWS" }
      

    filter "system:linux"
      system "linux"
      linkoptions {"-pthread"}
      disablewarnings {"int-conversion", "incompatible-pointer-types"}
      defines { "SW_PLATFORM_LINUX" }
      links { "GL" }
      

    filter "configurations:Debug"
      defines { "DEBUG" }
      runtime "Debug"
      symbols "On"

    filter "configurations:Release"
      defines { "RDEBUG" }
      runtime "Release"
      optimize "On"

    filter "configurations:Dist"
      defines { "NDEBUG" }
      runtime "Release"
      optimize "On"


