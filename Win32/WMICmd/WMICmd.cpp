////////////////////////////////////////////////////////////////////////////////
//! \file   WmiCmd.cpp
//! \brief  The WmiCmd class definition.
//! \author Chris Oldwood

#include "Common.hpp"
#include "WmiCmd.hpp"
#include <WCL/Path.hpp>
#include <WCL/VerInfoReader.hpp>
#include "CmdLineArgs.hpp"
#include <Core/CmdLineException.hpp>
#include <Core/StringUtils.hpp>
#include <WCL/AutoCom.hpp>
#include "QueryCmd.hpp"

////////////////////////////////////////////////////////////////////////////////
// Global variables.

//! The application object.
WmiCmd g_app;

////////////////////////////////////////////////////////////////////////////////
// Local variables.

static tstring s_appName(TXT("WMICmd"));

////////////////////////////////////////////////////////////////////////////////
// The table of command line switches.

static Core::CmdLineSwitch s_switches[] = 
{
	{ USAGE,	TXT("?"),	NULL,			Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	NULL										},
	{ USAGE,	TXT("h"),	TXT("help"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program options syntax")	},
	{ VERSION,	TXT("v"),	TXT("version"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the program version")			},
	{ MANUAL,	NULL,		TXT("manual"),	Core::CmdLineSwitch::ONCE,	Core::CmdLineSwitch::NONE,	NULL,	TXT("Display the manual")					},
};
static size_t s_switchCount = ARRAY_SIZE(s_switches);

////////////////////////////////////////////////////////////////////////////////
//! Default constructor.

WmiCmd::WmiCmd()
	: m_parser(s_switches, s_switches+s_switchCount)
{
}

////////////////////////////////////////////////////////////////////////////////
//! Destructor.

WmiCmd::~WmiCmd()
{
}

////////////////////////////////////////////////////////////////////////////////
//! Run the application.

int WmiCmd::run(int argc, tchar* argv[], tistream& /*in*/, tostream& out, tostream& err)
{
	// Command specified?
	if ( (argc > 1) && ((argv[1][0] != TXT('/')) && (argv[1][0] != TXT('-'))) )
	{
		// Initilise COM layer.
		WCL::AutoCom com(COINIT_APARTMENTTHREADED);
/*
		HRESULT result = ::CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
												RPC_C_AUTHN_LEVEL_DEFAULT,
												RPC_C_IMP_LEVEL_IMPERSONATE,
												nullptr, EOAC_NONE, nullptr);

		if (FAILED(result))
			throw WCL::ComException(result, TXT("Failed to initialize default COM security settings"));
*/
		// Get command and execute.
		CommandPtr command = createCommand(argc, argv);

		return command->execute(out, err);
	}

	m_parser.parse(argc, argv, Core::CmdLineParser::ALLOW_ANY_FORMAT);

	// Request for command line syntax?
	if (m_parser.isSwitchSet(USAGE))
	{
		showUsage(out);
		return EXIT_SUCCESS;
	}
	// Request for version?
	else if (m_parser.isSwitchSet(VERSION))
	{
		showVersion(out);
		return EXIT_SUCCESS;
	}
	// Request for the manual?
	else if (m_parser.isSwitchSet(MANUAL))
	{
		showManual(err);
		return EXIT_SUCCESS;
	}

	throw Core::CmdLineException(TXT("No command specified"));
}

////////////////////////////////////////////////////////////////////////////////
//! Create the Comand object.

CommandPtr WmiCmd::createCommand(int argc, tchar* argv[])
{
	ASSERT(argc > 1);

	// Validate command line.
	const tchar* command = argv[1];

	// Create command.
	if (tstricmp(command, TXT("query")) == 0)
	{
		return CommandPtr(new QueryCmd(argc, argv));
	}

	throw Core::CmdLineException(Core::fmt(TXT("Unknown command: '%s'"), command));
}

////////////////////////////////////////////////////////////////////////////////
//! Display the program options syntax.

void WmiCmd::showUsage(tostream& out)
{
	out << std::endl;
	out << TXT("USAGE: ") << s_appName << (" <command> [options] ...") << std::endl;
	out << std::endl;

	size_t width = 16;

	out << TXT("where <command> is one of:-") << std::endl;
	out << std::endl;
	out << TXT("query") << tstring(width-5, TXT(' ')) << ("Execute a query") << std::endl;
	out << std::endl;

	out << TXT("For help on an individual command use:-") << std::endl;
	out << std::endl;
	out << s_appName << TXT(" <command> -?") << std::endl;
	out << std::endl;

	out << TXT("Non-command options:-") << std::endl;
	out << std::endl;
	out << m_parser.formatSwitches(Core::CmdLineParser::UNIX);
}

////////////////////////////////////////////////////////////////////////////////
//! Display the program version.

void WmiCmd::showVersion(tostream& out)
{
	// Extract details from the resources.
	tstring filename  = CPath::Application();
	tstring version   = WCL::VerInfoReader::GetStringValue(filename, WCL::VerInfoReader::PRODUCT_VERSION);
	tstring copyright = WCL::VerInfoReader::GetStringValue(filename, WCL::VerInfoReader::LEGAL_COPYRIGHT);

#ifdef _DEBUG
	version += TXT(" [Debug]");
#endif

	// Display version etc.
	out << std::endl;
	out << s_appName << TXT(" v") << version << std::endl;
	out << std::endl;
	out << copyright << std::endl;
	out << TXT("gort@cix.co.uk") << std::endl;
	out << TXT("www.cix.co.uk/~gort") << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
//! Display the manual.

void WmiCmd::showManual(tostream& err)
{
	// Look for .mht based helpfile first.
	tstring helpfile_mht = s_appName + TXT(".mht");
	CPath   fullpath_mht = CPath::ApplicationDir() / helpfile_mht.c_str();

	if (fullpath_mht.Exists())
	{
		::ShellExecute(NULL, NULL, fullpath_mht.c_str(), NULL, NULL, SW_SHOW);
	}

	// Fall back to .html based helpfile.
	tstring helpfile_html = s_appName + TXT(".html");
	CPath   fullpath_html = CPath::ApplicationDir() / helpfile_html.c_str();

	if (fullpath_html.Exists())
	{
		::ShellExecute(NULL, NULL, fullpath_html, NULL, NULL, SW_SHOW);
	}

	err << TXT("ERROR: Manual missing - '") << fullpath_html.c_str() << TXT("'") << std::endl;
}
