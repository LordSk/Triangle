--
dofile("config.lua");

PROJ_DIR = path.getabsolute("..")
BUILD_DIR = path.join(PROJ_DIR, "build")

solution "Triangle solution"
	location "build"
	
	configurations {
		"Debug",
		"Release"
	}

	platforms {
		"x64"
	}
	
	language "C++"
	
	configuration {"Debug"}
		targetsuffix "_debug"
		flags {
			"Symbols"
		}
		defines {
			"DEBUG",
			"CONF_DEBUG"
		}
        links {
            bimg_lib_debug,
            bx_lib_debug,
            bgfx_lib_debug,
        }
	
	configuration {"Release"}
		targetsuffix "_release"
		flags {
			"Optimize"
		}
		defines {
			"NDEBUG",
			"CONF_RELEASE"
		}
        links {
            bimg_lib_release,
            bx_lib_release,
            bgfx_lib_release,
        }
	
	configuration {}
	
	targetdir(BUILD_DIR)
	
	includedirs {
		"src",
		SDL2_include,
		bx_include,
		bgfx_include,
	}
	
	links {
		"user32",
		"shell32",
		"winmm",
		"ole32",
		"oleaut32",
		"imm32",
		"version",
		"ws2_32",
		"advapi32",
        "comdlg32", -- GetOpenFileName 
		"gdi32",
		"glu32",
		"opengl32",
		SDL2_lib,
	}
	
	flags {
		"NoExceptions",
		"NoRTTI",
		"EnableSSE",
		"EnableSSE2",
		"EnableAVX",
		"EnableAVX2",
	}
	
	defines {
	}
	
	-- disable exception related warnings
	buildoptions{ "/wd4577", "/wd4530" }
	

project "Triangle"
	kind "WindowedApp"
	
	configuration {}
	
	files {
		"src/**.h",
		"src/**.c",
		"src/**.cpp",
	}
    
    linkoptions{ "/subsystem:windows" }