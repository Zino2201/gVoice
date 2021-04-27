#include "GarrysMod/Lua/Interface.h"
#include <msclr/marshal.h>
#include <queue>
#include <string>
#include <fmt/core.h>

#using "System.Speech.dll"
#using "System.Runtime.dll"
#using "System.Runtime.InteropServices.dll"

constexpr int gvoice_major_version = 1;
constexpr int gvoice_minor_version = 0;
constexpr int gvoice_patch_version = 0;

using namespace GarrysMod::Lua;
using namespace System;
using namespace System::Speech::Synthesis;
using namespace System::Speech::Recognition;

/** Globals gc variables */
ref class Globals
{
public:
	static SpeechSynthesizer^ synthesizer = nullptr;
	static SpeechRecognitionEngine^ engine = nullptr;
	static Grammar^ grammar = nullptr;
	static Globalization::CultureInfo^ culture = nullptr;
	static msclr::interop::marshal_context^ marshal_context;
};

/** Global speech queue */
std::queue<std::string> recognition_queue;
bool is_listening = false;

/** 
 * Logging API
 */
namespace gvlog
{

struct LogColor
{
	double r, g, b, a;
};

void Log(GarrysMod::Lua::ILuaBase* LUA, const std::string& msg, const LogColor& in_color)
{
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->GetField(-1, "hook");
		LUA->GetField(-1, "Run");
		LUA->PushString("GVoice.LogImpl");
		LUA->PushString(msg.c_str());
		LUA->PushNumber(in_color.r);
		LUA->PushNumber(in_color.g);
		LUA->PushNumber(in_color.b);
		LUA->PushNumber(in_color.a);
		LUA->PCall(6, 0, 0);
	LUA->Pop(2);
}

template<typename... Args>
inline void Info(GarrysMod::Lua::ILuaBase* LUA, const std::string& fmt, Args&&... args)
{
	Log(LUA, fmt::format(fmt, std::forward<Args>(args)...), { 255, 255, 255, 255 });
}

template<typename... Args>
inline void Warn(GarrysMod::Lua::ILuaBase* LUA, const std::string& fmt, Args&&... args)
{
	Log(LUA, fmt::format(fmt, std::forward<Args>(args)...), { 255, 255, 0, 255 });
}

template<typename... Args>
inline void Error(GarrysMod::Lua::ILuaBase* LUA, const std::string& fmt, Args&&... args)
{
	Log(LUA, fmt::format(fmt, std::forward<Args>(args)...), { 255, 90, 90, 255 });
}

}

/** 
 * Function to convert a .NET string (UTF-16) to a UTF-8 std::string
 */
std::string ConvertNETStringToUTF8(String^ str16)
{
	array<Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(str16);

	std::string str;
	str.resize(bytes->Length);
	System::Runtime::InteropServices::Marshal::Copy(bytes, 0, IntPtr(const_cast<char*>(str.data())), bytes->Length);
	return str;
}

/**
 * Called when speech is recognized, push to the queue so the next call to UpdateRecognition gets it
 */
void OnSpeechRecognized(Object^ sender, SpeechRecognizedEventArgs^ args)
{
	recognition_queue.push(ConvertNETStringToUTF8(args->Result->Text));
}

LUA_FUNCTION(FlushRecognitionQueue)
{
	while(!recognition_queue.empty())
	{
		const auto& data = recognition_queue.front();

		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			LUA->GetField(-1, "hook");
			LUA->GetField(-1, "Run");
			LUA->PushString("GVoice.OnSpeechRecognized");
			LUA->PushString(data.c_str());
			LUA->PCall(2, 0, 0);
		LUA->Pop(2);

		recognition_queue.pop();
	}

	return 0;
}

/** 
 * Text To Speach
 * \param str : What to speaks
 */
LUA_FUNCTION(Speak)
{
	const char* str = LUA->CheckString(1);

	String^ cs_str = gcnew String(str);
	Globals::synthesizer->Rate = 1;
	Globals::synthesizer->Volume = 100;
	Globals::synthesizer->SpeakAsync(cs_str);

	return 0;
}

/** 
 * Init recognition engine
 * \return Used culture
 */
LUA_FUNCTION(InitRecognition)
{
	/** Clear old things if required */
	{
		recognition_queue = {};
		Globals::marshal_context = gcnew msclr::interop::marshal_context();

		if(Globals::engine)
		{	
			Globals::engine->RecognizeAsyncStop();
			Globals::engine->UnloadAllGrammars();
		}
	}
	
	gvlog::Info(LUA, "Initializing GVoice Recognition Engine");

	const char* culture = LUA->CheckString(1);
	const char* ret_culture = culture;

	/** Create the speech recognition engine */
	try
	{
		String^ clr_culture_name = gcnew String(culture);
		Globals::culture = gcnew Globalization::CultureInfo(clr_culture_name);
		Globals::engine = gcnew SpeechRecognitionEngine(Globals::culture);
	}
	catch(Exception^)
	{
		gvlog::Error(LUA, "Unsupported culture {}, please install the correct language pack for Windows. Fallbacking to default one.", culture);
		Globals::culture = Globalization::CultureInfo::CurrentCulture;
		Globals::engine = gcnew SpeechRecognitionEngine(Globals::culture);

		/** Marshal Name */
		ret_culture = Globals::marshal_context->marshal_as<const char*>(Globals::culture->Name);
	}

	/** Use default input device*/
	Globals::engine->SetInputToDefaultAudioDevice();
	
	/** Register events */
	Globals::engine->SpeechRecognized += gcnew EventHandler<SpeechRecognizedEventArgs^>(&OnSpeechRecognized);

	/** Build the grammar if provided */
	if(LUA->IsType(2, GarrysMod::Lua::Type::Table))
	{
		Choices^ choices = gcnew Choices();

		LUA->PushNil();
		while(LUA->Next(2) != 0)
		{
			const char* value = LUA->GetString(-1);
			choices->Add(gcnew String(value));
			LUA->Pop();	
		}

		GrammarBuilder^ builder = gcnew GrammarBuilder();
		builder->Culture = Globals::culture;
		builder->Append(choices);

		try
		{
			Globals::grammar = gcnew Grammar(builder);
		}
		catch(Exception^) {}
	}
	else
	{
		Globals::grammar = gcnew DictationGrammar();
	}

	/**
	 * Load the builded grammar
	 */
	if(Globals::grammar)
	{
		Globals::engine->LoadGrammar(Globals::grammar);
	}
	else
	{
		gvlog::Error(LUA, "Null grammar! Aborting initialization.");
		LUA->PushString("");
		return 1;
	}

	gvlog::Info(LUA, "Using culture {}", ret_culture);

	LUA->PushString(ret_culture);
	return 1;
}

LUA_FUNCTION(StartRecognition)
{
	if(Globals::engine && Globals::grammar)
	{
		is_listening = true;
		Globals::engine->RecognizeAsync(RecognizeMode::Multiple);
		LUA->PushBool(true);
	}
	LUA->PushBool(false);

	return 0;
}

LUA_FUNCTION(StopRecognition)
{
	if(Globals::engine && Globals::grammar)
	{
		is_listening = false;
		Globals::engine->RecognizeAsyncStop();
		LUA->PushBool(true);
	}
	LUA->PushBool(false);

	return 0;
}

LUA_FUNCTION(IsListening)
{
	LUA->PushBool(is_listening);
	return 1;
}

GMOD_MODULE_OPEN()
{
	gvlog::Info(LUA, "=== gvoice v{}.{}.{} ===", 
		gvoice_major_version,
		gvoice_minor_version,
		gvoice_patch_version);

	/** Init synths things */
	Globals::synthesizer = gcnew SpeechSynthesizer();

	/** Main module table */
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "gvoice");
	if(!LUA->IsType(-1, GarrysMod::Lua::Type::Table))
	{
		LUA->Pop(1);
		LUA->CreateTable();
	}

	LUA->PushCFunction(Speak);
	LUA->SetField(-2, "Speak");
	LUA->PushCFunction(InitRecognition);
	LUA->SetField(-2, "InitRecognition");
	LUA->PushCFunction(StartRecognition);
	LUA->SetField(-2, "StartRecognition");
	LUA->PushCFunction(StopRecognition);
	LUA->SetField(-2, "StopRecognition");
	LUA->PushCFunction(FlushRecognitionQueue);
	LUA->SetField(-2, "FlushRecognitionQueue");
	LUA->PushCFunction(IsListening);
	LUA->SetField(-2, "IsListening");

	LUA->SetField(-2, "gvoice");

	return 0;
}

GMOD_MODULE_CLOSE()
{
	return 0;
}