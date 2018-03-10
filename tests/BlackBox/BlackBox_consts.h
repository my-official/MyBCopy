#pragma once


//const wstring g_OldMgrsDir = L"oldmgrs";

const vector<wstring> g_Backup1Files =
{
	 { L"File1.txt" }
	 , { L"Ctorrent.sdf" }
		 , { L"Ctorrent.sln" }
		 , { L"Ctorrent.vcxproj" }
		 , { L"Ctorrent.v12.suo" }
		 , { L"Ctorrent.vcxproj.filters" }
		 , { L"Ctorrent-Debug.vgdbsettings" }
		 , { L"Ctorrent-Release.vgdbsettings" }
		 , { L"gcc_Debug.h" }
		 , { L"gcc_Release.h" }
		 , { L"makefile" }
			 
		 , { L"oldsfmgr\\xml\\~user.xml" }
		 , { L"oldsfmgr\\~Makefile" }

		 , { L"oldsfmgr\\~prototype.cpp" }
		 , { L"oldsfmgr\\~tables.cpp" }
		 , { L"oldsfmgr\\~tables.h" }

		 , { L"sfchat\\Makefile" }
		 , { L"sfchat\\actions\\~generators.cPp" }
		 , { L"sfchat\\actions\\~settings.cpp" }

		 , { L"sfchat\\channels\\~binarychannel.cpp" }
		 , { L"sfchat\\channels\\~binarychannel.h" }
		 , { L"sfchat\\channels\\~chatchannel.cpp" }
		 , { L"sfchat\\channels\\~chatchannel.h" }
		 , { L"sfchat\\channels\\~discoverychannel.cpp" }
		 , { L"sfchat\\channels\\~discoverychannel.h" }
		,  { L"sfchat\\channels\\mgr.log" }

		 , { L"sfchat\\discovery\\behaviortree\\composite\\~composite.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\composite\\~composite.h" }
		 , { L"sfchat\\discovery\\behaviortree\\composite\\debug\\Win32\\MSVC2015\\t.log\\vgdb.log" }
		 , { L"sfchat\\discovery\\behaviortree\\decorator\\~decorator.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\decorator\\~decorator.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~behavior_tree.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\~behavior_tree.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~blackboard.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\~blackboard.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~common.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~control_node.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\~control_node.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~leaf.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\~leaf.h" }
		 , { L"sfchat\\discovery\\behaviortree\\~node.cpp" }
		 , { L"sfchat\\discovery\\behaviortree\\~node.h" }		 
		 , { L"sfchat\\discovery\\generators\\~generatorutils.h" }
		 , { L"sfchat\\discovery\\generators\\~linkbuilder.cpp" }
		 , { L"sfchat\\discovery\\generators\\~linkbuilder.h" }		 
		 , { L"sfchat\\discovery\\~analytics_discovery.cpp" }
		 , { L"sfchat\\discovery\\~analytics_discovery.h" }
		 , { L"sfchat\\discovery\\~asyncsystem.cpp" }
		 , { L"sfchat\\discovery\\~asyncsystem.h" }
		 , { L"sfchat\\discovery\\~dijkstra.cpp" }
		 , { L"sfchat\\discovery\\~dijkstra.h" }
		 , { L"sfchat\\discovery\\~discovery.cpp" }
		 , { L"sfchat\\discovery\\~discovery.h" }
		 , { L"sfchat\\discovery\\~discoveryaway.cpp" }
		 , { L"sfchat\\discovery\\~discoveryserver.cpp" }
		 , { L"sfchat\\discovery\\~discoveryserver.h" }
		 		 
		 , { L"sfchat\\xml\\~channel.xml" }
		 , { L"sfchat\\xml\\~connection.xml" }
		 , { L"sfchat\\xml\\~generators.xml" }
		 , { L"sfchat\\xml\\~settings.xml" }
		 , { L"sfchat\\xml\\~sfchat.xml" }
		 
		 , { L"sfchat\\~SFProtocol.h" }
		 , { L"sfchat\\~analytics_sfchat.cpp" }
		 , { L"sfchat\\~analytics_sfchat.h" }
		 , { L"sfchat\\~auth.cpp" }
		 , { L"sfchat\\~bufferregistrator.cpp" }
		 , { L"sfchat\\~bufferregistrator.h" }
		 
		 , { L"sfchat\\~server.cpp" }
		 , { L"sfchat\\~server.h" }
		 , { L"sfchat\\~sfchat.cpp" }
		 , { L"sfchat\\~sfchat.h" }
		 , { L"sfchat\\~sfchat.vcxproj" }
		 , { L"sfchat\\~sfchat.vcxproj.filters" }
};




const vector<wstring> g_Backup1FilesFiltred =
{	
	 { L"Ctorrent.sln" }			
	,{ L"sfchat\\Makefile" }	
	,{ L"sfchat\\actions\\~settings.cpp" }

	,{ L"sfchat\\channels\\~binarychannel.cpp" }
	,{ L"sfchat\\channels\\~binarychannel.h" }
	,{ L"sfchat\\channels\\~chatchannel.cpp" }
	,{ L"sfchat\\channels\\~chatchannel.h" }
	,{ L"sfchat\\channels\\~discoverychannel.cpp" }
	,{ L"sfchat\\channels\\~discoverychannel.h" }	

	,{ L"sfchat\\discovery\\behaviortree\\composite\\~composite.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\composite\\~composite.h" }
	,{ L"sfchat\\discovery\\behaviortree\\composite\\debug\\Win32\\MSVC2015\\t.log\\vgdb.log" }
	,{ L"sfchat\\discovery\\behaviortree\\decorator\\~decorator.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\decorator\\~decorator.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~behavior_tree.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\~behavior_tree.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~blackboard.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\~blackboard.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~common.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~control_node.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\~control_node.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~leaf.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\~leaf.h" }
	,{ L"sfchat\\discovery\\behaviortree\\~node.cpp" }
	,{ L"sfchat\\discovery\\behaviortree\\~node.h" }

	,{ L"sfchat\\discovery\\~asyncsystem.cpp" }
	,{ L"sfchat\\discovery\\~asyncsystem.h" }
	,{ L"sfchat\\discovery\\~dijkstra.cpp" }
	,{ L"sfchat\\discovery\\~dijkstra.h" }
	,{ L"sfchat\\discovery\\~discovery.cpp" }
	,{ L"sfchat\\discovery\\~discovery.h" }
	,{ L"sfchat\\discovery\\~discoveryaway.cpp" }
	,{ L"sfchat\\discovery\\~discoveryserver.cpp" }
	,{ L"sfchat\\discovery\\~discoveryserver.h" }

	,{ L"sfchat\\xml\\~channel.xml" }
	,{ L"sfchat\\xml\\~connection.xml" }
	,{ L"sfchat\\xml\\~generators.xml" }
	,{ L"sfchat\\xml\\~settings.xml" }
	,{ L"sfchat\\xml\\~sfchat.xml" }
	
	,{ L"sfchat\\~SFProtocol.h" }
	,{ L"sfchat\\~analytics_sfchat.cpp" }
	,{ L"sfchat\\~analytics_sfchat.h" }
	,{ L"sfchat\\~auth.cpp" }
	,{ L"sfchat\\~bufferregistrator.cpp" }
	,{ L"sfchat\\~bufferregistrator.h" }

	,{ L"sfchat\\~server.cpp" }
	,{ L"sfchat\\~server.h" }
	,{ L"sfchat\\~sfchat.cpp" }
	,{ L"sfchat\\~sfchat.h" }
};



const wstring g_NewMgrsDir = L"newmgrs";

const vector<wstring> g_NewMgrsFiles =
{
	 { L"CodeDB\\galaxymgr-Release-Win32\\AutoPCH\\71.ast" }
	,{ L"CodeDB\\galaxymgr-Release-Win32\\AutoPCH\\71.psf" }
	,{ L"CodeDB\\galaxymgr-Release-Win32\\GlobalFileTable.000" }	
};



const wstring g_CTorrentDir = L"ctorrent";

const vector<wstring> g_CTorrentFiles =
{
	 { L"Ctorrent-Debug.vgdbsettings" }
	,{ L"Ctorrent-Release.vgdbsettings" }
	,{ L"Ctorrent.lngservicesettings" }
	,{ L"Ctorrent.sdf" }
	,{ L"Ctorrent.sln" }
	,{ L"ctorrent-dnh3.3.2\\Makefile.in" }
	,{ L"ctorrent-dnh3.3.2\\NEWS" }
	,{ L"ctorrent-dnh3.3.2\\README" }

};


const wstring g_InputSrcXmlFile = L"BlackBoxTest_in.mybcopy";
const wstring g_OutputSrcXmlFile = L"BlackBoxTest_out.mybcopy";
