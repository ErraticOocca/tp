#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/JMath/JMath.h"
#include "dol2asm.h"
#include "m_Do/m_Do_mtx.h"

/* 8032EC28-8032ECAC 329568 0084+00 0/0 1/1 0/0 .text
 * init__25J3DMtxCalcJ3DSysInitBasicFRC3VecRA3_A4_Cf            */
void J3DMtxCalcJ3DSysInitBasic::init(Vec const& scale, Mtx const& mtx) {
    J3DSys::mCurrentS = scale;
    J3DSys::mParentS = (Vec){1.0f, 1.0f, 1.0f};
    JMAMTXApplyScale(mtx, J3DSys::mCurrentMtx, J3DSys::mCurrentS.x, J3DSys::mCurrentS.y,
                     J3DSys::mCurrentS.z);
}

/* 8032ECAC-8032ED30 3295EC 0084+00 0/0 2/2 2/2 .text
 * init__24J3DMtxCalcJ3DSysInitMayaFRC3VecRA3_A4_Cf             */
void J3DMtxCalcJ3DSysInitMaya::init(Vec const& scale, Mtx const& mtx) {
    J3DSys::mParentS = (Vec){1.0f, 1.0f, 1.0f};
    J3DSys::mCurrentS = scale;
    JMAMTXApplyScale(mtx, J3DSys::mCurrentMtx, J3DSys::mCurrentS.x, J3DSys::mCurrentS.y,
                     J3DSys::mCurrentS.z);
}

/* ############################################################################################## */
/* 804515F0-804515F4 000AF0 0004+00 3/3 1/1 0/0 .sbss            mMtxBuffer__10J3DMtxCalc */
J3DMtxBuffer* J3DMtxCalc::mMtxBuffer;

/* 804515F4-804515F8 000AF4 0004+00 4/4 9/9 2/2 .sbss            mJoint__10J3DMtxCalc */
J3DJoint* J3DMtxCalc::mJoint;

inline s32 checkScaleOne(const Vec& param_0) {
    if (param_0.x == 1.0f && param_0.y == 1.0f && param_0.z == 1.0f) {
        return true;
    } else {
        return false;
    }
}

/* 8032ED30-8032EE50 329670 0120+00 0/0 1/1 0/0 .text
 * calcTransform__28J3DMtxCalcCalcTransformBasicFRC16J3DTransformInfo */
void J3DMtxCalcCalcTransformBasic::calcTransform(J3DTransformInfo const& transInfo) {
    J3DMtxBuffer* mtxBuf = J3DMtxCalc::getMtxBuffer();
    u16 jntNo = J3DMtxCalc::getJoint()->getJntNo();

    MtxP anmMtx = mtxBuf->getAnmMtx(jntNo);

    J3DSys::mCurrentS.x *= transInfo.mScale.x;
    J3DSys::mCurrentS.y *= transInfo.mScale.y;
    J3DSys::mCurrentS.z *= transInfo.mScale.z;
    J3DGetTranslateRotateMtx(transInfo, anmMtx);

    if (!checkScaleOne(J3DSys::mCurrentS)) {
        mtxBuf->setScaleFlag(jntNo, 0);
        JMAMTXApplyScale(anmMtx, anmMtx, transInfo.mScale.x, transInfo.mScale.y,
                         transInfo.mScale.z);
    } else {
        mtxBuf->setScaleFlag(jntNo, 1);
    }

    MTXConcat(J3DSys::mCurrentMtx, anmMtx, J3DSys::mCurrentMtx);
    MTXCopy(J3DSys::mCurrentMtx, anmMtx);
}

/* 8032EE50-8032EFBC 329790 016C+00 0/0 1/1 0/0 .text
 * calcTransform__32J3DMtxCalcCalcTransformSoftimageFRC16J3DTransformInfo */
void J3DMtxCalcCalcTransformSoftimage::calcTransform(J3DTransformInfo const& transInfo) {
    J3DMtxBuffer* mtxBuf = J3DMtxCalc::getMtxBuffer();
    u16 jntNo = J3DMtxCalc::getJoint()->getJntNo();

    MtxP anmMtx = mtxBuf->getAnmMtx(jntNo);

    J3DGetTranslateRotateMtx(transInfo.mRotation.x, transInfo.mRotation.y, transInfo.mRotation.z,
                             transInfo.mTranslate.x * J3DSys::mCurrentS.x,
                             transInfo.mTranslate.y * J3DSys::mCurrentS.y,
                             transInfo.mTranslate.z * J3DSys::mCurrentS.z, anmMtx);
    MTXConcat(J3DSys::mCurrentMtx, anmMtx, J3DSys::mCurrentMtx);

    J3DSys::mCurrentS.x *= transInfo.mScale.x;
    J3DSys::mCurrentS.y *= transInfo.mScale.y;
    J3DSys::mCurrentS.z *= transInfo.mScale.z;

    if (!checkScaleOne(J3DSys::mCurrentS)) {
        mtxBuf->setScaleFlag(jntNo, 0);
        JMAMTXApplyScale(J3DSys::mCurrentMtx, anmMtx, J3DSys::mCurrentS.x, J3DSys::mCurrentS.y,
                         J3DSys::mCurrentS.z);
        anmMtx[0][3] = J3DSys::mCurrentMtx[0][3];
        anmMtx[1][3] = J3DSys::mCurrentMtx[1][3];
        anmMtx[2][3] = J3DSys::mCurrentMtx[2][3];
    } else {
        mtxBuf->setScaleFlag(jntNo, 1);
        MTXCopy(J3DSys::mCurrentMtx, anmMtx);
    }
}

/* 8032EFBC-8032F13C 3298FC 0180+00 0/0 6/6 2/2 .text
 * calcTransform__27J3DMtxCalcCalcTransformMayaFRC16J3DTransformInfo */
void J3DMtxCalcCalcTransformMaya::calcTransform(J3DTransformInfo const& transInfo) {
    J3DJoint* joint = J3DMtxCalc::getJoint();
    J3DMtxBuffer* mtxBuf = J3DMtxCalc::getMtxBuffer();

    u16 jntNo = joint->getJntNo();

    MtxP anmMtx = mtxBuf->getAnmMtx(jntNo);

    J3DGetTranslateRotateMtx(transInfo, anmMtx);

    if (transInfo.mScale.x == 1.0f && transInfo.mScale.y == 1.0f && transInfo.mScale.z == 1.0f) {
        mtxBuf->setScaleFlag(jntNo, 1);
    } else {
        mtxBuf->setScaleFlag(jntNo, 0);
        JMAMTXApplyScale(anmMtx, anmMtx, transInfo.mScale.x, transInfo.mScale.y,
                         transInfo.mScale.z);
    }

    if (joint->getScaleCompensate() == 1) {
        f32 invX = JMath::fastReciprocal(J3DSys::mParentS.x);
        f32 invY = JMath::fastReciprocal(J3DSys::mParentS.y);
        f32 invZ = JMath::fastReciprocal(J3DSys::mParentS.z);

        anmMtx[0][0] *= invX;
        anmMtx[0][1] *= invX;
        anmMtx[0][2] *= invX;
        anmMtx[1][0] *= invY;
        anmMtx[1][1] *= invY;
        anmMtx[1][2] *= invY;
        anmMtx[2][0] *= invZ;
        anmMtx[2][1] *= invZ;
        anmMtx[2][2] *= invZ;
    }

    MTXConcat(J3DSys::mCurrentMtx, anmMtx, J3DSys::mCurrentMtx);
    MTXCopy(J3DSys::mCurrentMtx, anmMtx);

    J3DSys::mParentS.x = transInfo.mScale.x;
    J3DSys::mParentS.y = transInfo.mScale.y;
    J3DSys::mParentS.z = transInfo.mScale.z;
}

/* 8032F13C-8032F170 329A7C 0034+00 0/0 1/1 0/0 .text            appendChild__8J3DJointFP8J3DJoint
 */
void J3DJoint::appendChild(J3DJoint* pChild) {
    if (mChild == NULL) {
        mChild = pChild;
    } else {
        J3DJoint* curChild = mChild;
        while (curChild->getYounger() != NULL) {
            curChild = curChild->getYounger();
        }
        curChild->setYounger(pChild);
    }
}

/* ############################################################################################## */

#pragma push
#pragma force_active on
/* dead data */
SECTION_DEAD static u32 const pad_803A2080[2] = {0, 0};
#pragma pop

/* 8032F170-8032F254 329AB0 00E4+00 0/0 1/1 0/0 .text            __ct__8J3DJointFv */
J3DJoint::J3DJoint() {
    mCallBackUserData = NULL;
    mCallBack = NULL;
    field_0x8 = NULL;
    mChild = NULL;
    mYounger = NULL;
    mJntNo = 0;
    mKind = 1;
    mScaleCompensate = false;
    __memcpy(&mTransformInfo, &j3dDefaultTransformInfo, sizeof(J3DTransformInfo));
    mBoundingSphereRadius = 0.0f;
    mMtxCalc = NULL;
    mMesh = NULL;

    Vec init = {0.0f, 0.0f, 0.0f};
    mMin = init;
    Vec init2 = {0.0f, 0.0f, 0.0f};
    mMax = init2;
}

/* 8032F254-8032F3F8 329B94 01A4+00 0/0 1/1 0/0 .text            entryIn__8J3DJointFv */
void J3DJoint::entryIn() {
    MtxP anmMtx = j3dSys.getModel()->i_getAnmMtx(mJntNo);
    j3dSys.getDrawBuffer(0)->setZMtx(anmMtx);
    j3dSys.getDrawBuffer(1)->setZMtx(anmMtx);
    for (J3DMaterial* mesh = mMesh; mesh != NULL;) {
        if (mesh->getShape()->checkFlag(1)) {
            mesh = mesh->getNext();
        } else {
            J3DMatPacket* matPacket = j3dSys.getModel()->getMatPacket(mesh->getIndex());
            J3DShapePacket* shapePacket =
                j3dSys.getModel()->getShapePacket(mesh->getShape()->getIndex());
            if (!matPacket->isLocked()) {
                if (mesh->getMaterialAnm()) {
                    J3DMaterialAnm* piVar8 = mesh->getMaterialAnm();
                    piVar8->calc(mesh);
                }
                mesh->calc(anmMtx);
            }
            mesh->setCurrentMtx();
            matPacket->setMaterialAnmID(mesh->getMaterialAnm());
            matPacket->setShapePacket(shapePacket);
            J3DDrawBuffer* drawBuffer = j3dSys.getDrawBuffer(mesh->isDrawModeOpaTexEdge());
            if ((u8)matPacket->entry(drawBuffer)) {
                j3dSys.setMatPacket(matPacket);
                J3DDrawBuffer::entryNum++;
                mesh->makeDisplayList();
            }
            mesh = mesh->getNext();
        }
    }
}

/* ############################################################################################## */
/* 804515F8-80451600 000AF8 0004+04 1/1 1/1 0/0 .sbss            mCurrentMtxCalc__8J3DJoint */
J3DMtxCalc* J3DJoint::mCurrentMtxCalc;

/* 8032F3F8-8032F5A8 329D38 01B0+00 0/0 1/1 0/0 .text            recursiveCalc__8J3DJointFv */
void J3DJoint::recursiveCalc() {
    J3DMtxCalc* prevMtxCalc = NULL;
    Mtx prevCurrentMtx;
    mDoMtx_copy(J3DSys::mCurrentMtx, prevCurrentMtx);
    f32 currentX = J3DSys::mCurrentS.x;
    f32 currentY = J3DSys::mCurrentS.y;
    f32 currentZ = J3DSys::mCurrentS.z;
    f32 parentX = J3DSys::mParentS.x;
    f32 parentY = J3DSys::mParentS.y;
    f32 parentZ = J3DSys::mParentS.z;
    if (getMtxCalc() != NULL) {
        prevMtxCalc = getCurrentMtxCalc();
        J3DMtxCalc* piVar2 = this->getMtxCalc();
        setCurrentMtxCalc(piVar2);
        J3DMtxCalc::setJoint(this);
        piVar2->calc();
    } else {
        if (getCurrentMtxCalc() != NULL) {
            J3DMtxCalc* uVar6 = getCurrentMtxCalc();
            J3DMtxCalc::setJoint(this);
            uVar6->calc();
        }
    }

    J3DJointCallBack jointCallback = getCallBack();
    if (jointCallback != NULL) {
        (*jointCallback)(this, 0);
    }

    J3DJoint* child = getChild();
    if (child != NULL) {
        child->recursiveCalc();
    }
    mDoMtx_copy(prevCurrentMtx, J3DSys::mCurrentMtx);

    J3DSys::mCurrentS.x = currentX;
    J3DSys::mCurrentS.y = currentY;
    J3DSys::mCurrentS.z = currentZ;
    J3DSys::mParentS.x = parentX;
    J3DSys::mParentS.y = parentY;
    J3DSys::mParentS.z = parentZ;

    if (prevMtxCalc != NULL) {
        setCurrentMtxCalc(prevMtxCalc);
    }
    if (jointCallback != NULL) {
        (*jointCallback)(this, 1);
    }

    J3DJoint* younger = getYounger();
    if (younger != NULL) {
        younger->recursiveCalc();
    }
}
