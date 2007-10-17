#include "ModelContainerView.h"

namespace VMAP
{
    char* gDataDir = NULL;
    char* gLogFile = NULL;
    //==========================================

    ModelContainerView::ModelContainerView(GApp* pApp) :  GApplet(pApp) {
        i_App = pApp;

        iCommandFileRW.setFileName(gLogFile);
        iCurrCmdIndex = 0;
        iVMapManager = new VMapManager();
        iDrawLine = false;

        iVARAreaRef = VARArea::create(1024*1024*60);
        iInstanceId = -1;
        iPosSent = false;

    }
    //===================================================

    ModelContainerView::~ModelContainerView(void)
    {
        Array<std::string > keys = iTriVarTable.getKeys();
        Array<std::string>::ConstIterator i = keys.begin();
        while(i != keys.end()) {
            VAR* var = iTriVarTable.get(*i);
            delete var;
            ++i;
        }
    }

    //===================================================

    void ModelContainerView::cleanup() {
    }

    //===================================================
    Vector3 getViewPos(const ModelContainer* mc, unsigned int pModelNr = 0) {
        if(mc->getNSubModel() < pModelNr) {
            pModelNr = mc->getNSubModel();
        }
        const SubModel sm = mc->getSubModel(pModelNr);
        return (sm.getAABoxBounds().low());
    }

    void ModelContainerView::init() {
    }

    //==========================================

    void fillSubModelArary(const ModelContainer* pModelContainer, const TreeNode *root, Array<SubModel>& array, Vector3& pLo, Vector3& pHi) {
        Vector3 lo = Vector3(inf(), inf(), inf());
        Vector3 hi = Vector3(-inf(), -inf(), -inf());

        for(int i=0; i<	root->getNValues(); i++) {
            SubModel sm = pModelContainer->getSubModel(root->getStartPosition() + i);
            lo = lo.min(sm.getAABoxBounds().low());
            hi = hi.max(sm.getAABoxBounds().high());
            array.append(sm);
        }

        if(root->getChild((TreeNode *) &pModelContainer->getTreeNode(0), 0)) {
            fillSubModelArary(pModelContainer, root->getChild((TreeNode *)&pModelContainer->getTreeNode(0), 0), array, lo, hi);
        }
        if(root->getChild((TreeNode *)&pModelContainer->getTreeNode(0), 1)) {
            fillSubModelArary(pModelContainer, root->getChild((TreeNode *)&pModelContainer->getTreeNode(0), 1), array, lo, hi);
        }

        float dist1 = (hi -lo).magnitude();
        AABox b;
        root->getBounds(b);
        float dist2 = (b.high() -b.low()).magnitude();
        if(dist1 > dist2) {
            // error
            int xxx = 0;
        }

    }

    //==========================================
    void ModelContainerView::addModelContainer(const std::string& pName,const ModelContainer* pModelContainer) {
        // VARArea::UsageHint::WRITE_EVERY_FEW_FRAMES

        int offset=0;

        Array<int> iIndexArray;
        Array<Vector3> iGlobArray;

        Array<SubModel> SMArray;
        Vector3 lo, hi;
        fillSubModelArary(pModelContainer, &pModelContainer->getTreeNode(0), SMArray,lo,hi);


        for(int i=0; i<SMArray.size(); ++i) {
            SubModel sm = SMArray[i];
            Array<Vector3> vArray;
            Array<int> iArray;
            fillVertexAndIndexArrays(sm, vArray, iArray);

            for(int j=0;j<iArray.size(); ++j) {
                iIndexArray.append(offset+iArray[j]);

            }
            for(int j=0;j<vArray.size(); ++j) {
                iGlobArray.append(vArray[j]);
            }
            offset += vArray.size();
            //break;
        }
        iTriVarTable.set(pName, new VAR(iGlobArray ,iVARAreaRef));
        iTriIndexTable.set(pName, iIndexArray);
    }

    //===================================================

    void ModelContainerView::removeModelContainer(const std::string& pName, const ModelContainer* pModelContainer) {
        VAR* var = iTriVarTable.get(pName);
        iTriVarTable.remove(pName);
        delete var;
    }

    Vector3 p1,p2,p3,p4,p5,p6,p7;
    Array<AABox>gBoxArray;
    int gCount1 = 0, gCount2 = 0 , gCount3 = 0, gCount4 = 0;
    bool myfound=false;

    //===================================================

    void ModelContainerView::doGraphics() {
        i_App->renderDevice->clear();
        RenderDevice *rd = i_App->renderDevice;

        rd->setProjectionAndCameraMatrix(i_App->debugCamera);

        LightingParameters lighting(GameTime(toSeconds(10, 00, AM)));
        //i_SkyRef->render(rd,lighting);
        rd->setAmbientLightColor(Color4(Color3::blue()));
        rd->enableLighting();

        GLight light =GLight::directional(i_App->debugController.getPosition() + i_App->debugController.getLookVector()*2,Color3::white());
        rd->setLight(0,light);


        Array<std::string > keys = iTriVarTable.getKeys();
        Array<std::string>::ConstIterator i = keys.begin();
        while(i != keys.end()) {
            VAR* var = iTriVarTable.get(*i);
            Array<int> indexArray = iTriIndexTable.get(*i);

            rd->beginIndexedPrimitives();
            rd->setVertexArray(*var);
            rd->sendIndices(RenderDevice::LINES, indexArray);
            rd->endIndexedPrimitives();
            ++i;
        }
        i_App->renderDevice->disableLighting();

        for(int i=0; i<gBoxArray.size(); ++i) {
            AABox b = gBoxArray[i];
            Draw::box(b,rd,Color3::red());
        }

        if(iDrawLine) {
            Draw::lineSegment(LineSegment::fromTwoPoints(iPos1, iPos2), rd, iColor, 3);

            if(myfound) {
                Draw::lineSegment(LineSegment::fromTwoPoints(p1, p2), rd, iColor, 3);
                Draw::lineSegment(LineSegment::fromTwoPoints(p2, p3), rd, iColor, 3);
                Draw::lineSegment(LineSegment::fromTwoPoints(p3, p1), rd, iColor, 3);
                Draw::sphere(Sphere(p4,0.5),rd, iColor);
                Draw::sphere(Sphere(p5,0.5),rd, Color3::green());
            }
        }
    }

    //===================================================

    void ModelContainerView::fillRenderArray(const SubModel& pSm, Array<TriangleBox> &pArray, const TreeNode* pTreeNode) {
        for(int i=0;i<pTreeNode->getNValues(); i++) {
            pArray.append(pSm.getTriangles()[i+pTreeNode->getStartPosition()]);
        }

        if(pTreeNode->getChild(pSm.getTreeNodes(), 0) != 0) {
            fillRenderArray(pSm, pArray, pTreeNode->getChild(pSm.getTreeNodes(), 0));
        }

        if(pTreeNode->getChild(pSm.getTreeNodes(), 1) != 0) {
            fillRenderArray(pSm, pArray, pTreeNode->getChild(pSm.getTreeNodes(), 1));
        }
    }

    //===================================================

    void ModelContainerView::fillVertexAndIndexArrays(const SubModel& pSm, Array<Vector3>& vArray, Array<int>& iArray) {
        Array<TriangleBox> tbarray;

        fillRenderArray(pSm, tbarray, &pSm.getTreeNode(0));
        MeshBuilder builder;
        int len = tbarray.size();
        int count = 0;
        for(int i=0;i<len;++i) {
            Triangle t = Triangle(tbarray[i].vertex(0).getVector3() + pSm.getBasePosition(),
                tbarray[i].vertex(1).getVector3() + pSm.getBasePosition(),
                tbarray[i].vertex(2).getVector3() + pSm.getBasePosition());

            vArray.append(t.vertex(0));
            vArray.append(t.vertex(1));
            vArray.append(t.vertex(2));

            iArray.append(count+0);
            iArray.append(count+1);
            iArray.append(count+1);
            iArray.append(count+2);
            iArray.append(count+2);
            iArray.append(count+0);
            count+=3;
        }
    }

    //====================================================================

    void ModelContainerView::showMap(int pMapId, int x, int y) {
        MapTree* mt = iVMapManager->getInstanceMapTree(pMapId);
        std::string dirFileName = iVMapManager->getDirFileName(pMapId);
        if(!mt->hasDirFile(dirFileName)) {
            dirFileName = iVMapManager->getDirFileName(pMapId, x, y);
        }
        showMap(mt,dirFileName);
        iInstanceId = pMapId;
    }

    //====================================================================

    bool ModelContainerView::loadAndShowTile(int pMapId, int x, int y) {
        char buffer[500];
        sprintf(buffer, "%s/vmaps",gDataDir);
        bool result = false;
        //if(pMapId == 1) return true; //+++
        int val = iVMapManager->loadMap(buffer,(unsigned int) pMapId, x,y);
        if(val == VMAP_LOAD_RESULT_OK) {
            result = true;
            showMap(pMapId,x,y);
        } else {
            printf("Unable to load %s\n", buffer);
        }
        return(result);
    }

    //=======================================================================

    void ModelContainerView::showMap(MapTree* mt, std::string dirFileName) {
        if(mt->hasDirFile(dirFileName)) {
            FilesInDir filesInDir = mt->getDirFiles(dirFileName);
            if(filesInDir.getRefCount() == 1) {
                Array<std::string> fileNames = filesInDir.getFiles();
                for(int i=0; i<fileNames.size(); ++i) {
                    std::string name = fileNames[i];
                    ManagedModelContainer* mc = mt->getModelContainer(name);
                    //if(mc->getNSubModel() == 791) {
                    addModelContainer(name, mc);
                    //}
                }
            }
        }
    }

    //=======================================================================

    bool ModelContainerView::loadAndShowTile(int pMapId) {
        char buffer[500];
        sprintf(buffer, "%s/vmaps",gDataDir);
        bool result = false;
        int val = iVMapManager->loadMap(buffer, (unsigned int) pMapId,-1,-1);
        if(val == VMAP_LOAD_RESULT_OK) {
            result = true;
            MapTree* mt = iVMapManager->getInstanceMapTree(pMapId);
            std::string dirFileName = iVMapManager->getDirFileName(pMapId);
            iTriVarTable = Table<std::string , VAR*>();
            iTriIndexTable = Table<std::string , Array<int> >(); // reset table
            iInstanceId = pMapId;
            showMap(mt,dirFileName);
        }
        return(result);
    }

    //====================================================================

    void ModelContainerView::processCommand() {
        iDrawLine = false;
        if(iCurrCmdIndex < iCmdArray.size()) {
            bool cmdfound = false;
            while(!cmdfound && (iCurrCmdIndex < iCmdArray.size())) {
                Command c = iCmdArray[iCurrCmdIndex];
                if(c.getType() == LOAD_TILE) {
                    iPrevLoadCommands.push_back(c);
                    if(iPosSent) {
                        if(loadAndShowTile(c.getInt(2),  c.getInt(0), c.getInt(1))) {
                            printf("load tile mapId=%d, %d, %d\n", c.getInt(2), c.getInt(0), c.getInt(1));
                        } else {
                            printf("ERROR: unable to load tile mapId= %d, %d, %d\n", c.getInt(2), c.getInt(0), c.getInt(1));
                        }
                        cmdfound = true;
                    } else {
                        printf("ignore load tile mapId=%d, %d, %d\n", c.getInt(2), c.getInt(0), c.getInt(1));
                    }
                } else if(c.getType() == LOAD_INSTANCE) {
                    if(loadAndShowTile(c.getInt(0))) {
                        printf("load instance %d\n", c.getInt(0));
                    }
                    cmdfound = true;
                } else if(c.getType() == UNLOAD_TILE) {
                    /*
                    iVMapManager->unloadMap(c.getInt(2), c.getInt(0), c.getInt(1));
                    printf("unload tile %d, %d\n", c.getInt(0), c.getInt(1));

                    std::string dirFileName = iVMapManager->getDirFileName(iVMapManager->getMapIdNames(c.getInt(2)).iMapGroupName.c_str(), c.getInt(0), c.getInt(1));
                    MapTree* mt = iVMapManager->getInstanceMapTree(c.getInt(2));
                    if(mt->hasDirFile(dirFileName)) {
                    Array<std::string> fileNames = mt->getDirFiles(dirFileName).getFiles();
                    for(int i=0; i<fileNames.size(); ++i) {
                    std::string name = fileNames[i];
                    ManagedModelContainer* mc = mt->getModelContainer(name);
                    removeModelContainer(name, mc);
                    }
                    }
                    */
                } else if(c.getType() == SET_POS) {
                    if(!iPosSent) {
                        int count = 3;
                        while(iPrevLoadCommands.size() > 0 && count>0) {
                            Command lc = iPrevLoadCommands.last();
                            iPrevLoadCommands.pop_back();
                            // first time, load the last map needed
                            if(loadAndShowTile(lc.getInt(2),  lc.getInt(0), lc.getInt(1))) {
                                printf("load tile mapid=%d, %d, %d\n", lc.getInt(2), lc.getInt(0), lc.getInt(1));
                            } else {
                                printf("ERROR: unable to load tile  mapid=%d, %d, %d\n", lc.getInt(2), lc.getInt(0), lc.getInt(1));
                            }
                            --count;
                        }
                    }
                    iPosSent = true;
                    i_App->debugController.setPosition(Vector3(c.getVector(0).x,c.getVector(0).y+3, c.getVector(0).z));
                    printf("set pos to %f, %f, %f\n",c.getVector(0).x, c.getVector(0).y, c.getVector(0).z );
                    cmdfound = true;
                } else if(c.getType() == TEST_VIS) {
                    printf("TEST line of sight\n");
                    iDrawLine = true;
                    iPos1 = iVMapManager->convertPositionToInternalRep(c.getVector(0).x, c.getVector(0).y, c.getVector(0).z);
                    iPos2 = iVMapManager->convertPositionToInternalRep(c.getVector(1).x, c.getVector(1).y, c.getVector(1).z);
                    if(c.getInt(0) != 0) {
                        iColor = Color3::green();
                    } else {
                        iColor = Color3::red();
                    }
                    cmdfound = true;

                }
                ++iCurrCmdIndex;
            }
        } else {
            printf("end reached\n");
        }
    }

    //====================================================================

    Vector3 ModelContainerView::convertPositionToMangosRep(float x, float y, float z) const {
        float pos[3];
        pos[0] = z;
        pos[1] = x;
        pos[2] = y;
        double full = 64.0*533.33333333;
        double mid = full/2.0;
        pos[0] = -((mid+pos[0])-full);
        pos[1] = -((mid+pos[1])-full);

        return(Vector3(pos));
    }


    //====================================================================


    void ModelContainerView::onUserInput(UserInput* ui) {

        if (ui->keyPressed(SDLK_ESCAPE)) {
            // Even when we aren't in debug mode, quit on escape.
            endApplet = true;
            i_App->endProgram = true;
        }

        if(ui->keyPressed(SDLK_l)) { //load
            iCmdArray = Array<Command>();
            iCommandFileRW.getNewCommands(iCmdArray);
            iCurrCmdIndex = 0;
            processCommand();
        }

        if(ui->keyPressed(SDLK_r)) { //restart
            iCurrCmdIndex = 0;
        }

        if(ui->keyPressed(SDLK_1)) { //inc count1
            gCount1++;
        }

        if(ui->keyPressed(SDLK_h)) { //inc count1
            i_App->debugController.getPosition();
            Vector3 pos = i_App->debugController.getPosition();
            Vector3 pos2 = convertPositionToMangosRep(pos.x, pos.y, pos.z);
            //Vector3 pos3 = iVMapManager->convertPositionToInternalRep(pos2.x, pos2.y, pos2.z);
            //pos3 = iVMapManager->convertPositionToInternalRep(pos2.x, pos2.y, pos2.z);

            float hight = iVMapManager->getHeight(iInstanceId, pos2.x, pos2.y, pos2.z);
            printf("Hight = %f\n",hight);
        }


        if(ui->keyPressed(SDLK_2)) { //dec count1
            gCount1--;
            if(gCount1 < 0) 
                gCount1 = 0;
        }

        if(ui->keyPressed(SDLK_z)) { //zero pos
            i_App->debugController.setPosition(Vector3(0,0,0));
            printf("set pos to 0, 0, 0\n");
        }


        if(ui->keyPressed(SDLK_c)) { //restart
            if(iCurrCmdIndex > 0) {
                if(iCmdArray[iCurrCmdIndex-1].getType() == TEST_VIS) {
                    Vector3 p1 = iCmdArray[iCurrCmdIndex-1].getVector(0);
                    Vector3 p2 = iCmdArray[iCurrCmdIndex-1].getVector(1);
                    bool result;
                    int mapId = iCmdArray[iCurrCmdIndex-1].getInt(1);
                    gCount3 = gCount2 = 0;// debug counter
                    gBoxArray = Array<AABox>();
                    result = iVMapManager->isInLineOfSight(mapId, p1.x,p1.y,p1.z,p2.x,p2.y,p2.z);
                    printf("recalc last line of light: result = %d\n", result);
                }
            }
        }


        if(ui->keyPressed(SDL_LEFT_MOUSE_KEY)) {
            if(		iCurrCmdIndex>0) {
                --iCurrCmdIndex;
                printf("restart last command\n");
                processCommand();
            }
        }

        GApplet::onUserInput(ui);

        if(ui->keyPressed(SDL_RIGHT_MOUSE_KEY)) {
            processCommand();
        }
    }
    //==========================================

    void ModelContainerView::setViewPosition(const Vector3& pPosition) {
        i_App->debugController.setPosition(pPosition);
        i_App->debugCamera.setPosition(pPosition);
    }

    //==========================================
    //==========================================
}

int main(int argc, char** argv) {
    if(argc == 3) {
        VMAP::gDataDir = argv[1];
        VMAP::gLogFile = argv[2];
        VMAP::ViewApp::startup();
    } else {
        printf("%s <data dir> <vmapcmd.log file>\n",argv[0]);
    }
}
