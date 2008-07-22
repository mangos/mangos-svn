#ifndef _MODELCONTAINERVIEW_H
#define _MODELCONTAINERVIEW_H

#include "ModelContainer.h"
#include "DebugCmdLogger.h"
#include "vmapmanager.h"



#include <G3DAll.h>

namespace VMAP
{
	//==========================================


	//==========================================

	class ModelContainerView : 
		public G3D::GApplet
	{
	private:
		VARAreaRef iVARAreaRef;
		Table<std::string , VAR*> iTriVarTable;
		Table<std::string , Array<int> > iTriIndexTable;
		//Array<int> iLineIndexArray;

		GApp* i_App;
		CommandFileRW iCommandFileRW;
		Array<Command> iCmdArray;
		int iCurrCmdIndex;

		VMapManager* iVMapManager;

		Vector3 iPos1;
		Vector3 iPos2;
		Color3 iColor;
		bool iDrawLine;
		int iInstanceId;
        bool iPosSent;
        Array<Command> iPrevLoadCommands;
	private:
		Vector3 convertPositionToMangosRep(float x, float y, float z) const;

	public:

		ModelContainerView(GApp* pApp);
		~ModelContainerView(void);

		void addModelContainer(const std::string& pName,const ModelContainer* pModelContainer);
		void removeModelContainer(const std::string& pName, const ModelContainer* pModelContainer);
		void setViewPosition(const Vector3& pPosition);

		void doGraphics();
		void init();
		void cleanup();
		void onUserInput(UserInput* ui);

		void fillRenderArray(const SubModel& pSm,Array<TriangleBox> &pArray, const TreeNode* pTreeNode);
		void fillVertexAndIndexArrays(const SubModel& pSm, Array<Vector3>& vArray, Array<int>& iArray);

		bool loadAndShowTile(int pMapId, int x, int y);
		void showMap(int pMapId, int x, int y);

		void showMap(MapTree* mt, std::string dirFileName);
		bool loadAndShowTile(int pMapId);


		void processCommand();

	};

	//==========================================
	//==========================================

	class ViewApp :  public G3D::GApp {
	private:
		ModelContainerView *iView;
	public:
		ViewApp(const GAppSettings& settings): GApp(settings) {}; 

		static void ViewApp::startup() {
			GAppSettings settings;
			settings.window.width = 1024;
			settings.window.height = 768;

			ViewApp* app = new ViewApp(settings);
			app->setDebugMode(true); 
			app->debugController.setActive(true);
			app->iView = new ModelContainerView(app);
			app->run();
		}

		void ViewApp::main() { iView->run(); }
	};

	//==========================================

	//==========================================



}

#endif
