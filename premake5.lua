workspace "DriftRemake"
architecture "x64"
    configurations { "Debug", "Release" }
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Core"
    location "%{prj.name}"
    kind "WindowedApp"
    language "C++"
    language "C++"
    targetname "drift"
    targetdir ("bin/".. outputdir)
    objdir ("%{prj.name}/int/" .. outputdir)
    cppdialect "C++17"
    staticruntime "Off"

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "%{prj.name}/**.hpp"
,        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "%{prj.name}/include",
        "%{prj.name}/src"
    }

    libdirs "%{prj.name}/lib"
	
	links
	{
	    "d3dcompiler",
		"d3d11"
	}

    filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }

	filter "configurations:Debug"
		defines { "_DEBUG", "_CONSOLE" }
		symbols "On"

    filter "configurations:Release"
		defines { "NDEBUG", "NCONSOLE" }
		optimize "On"
