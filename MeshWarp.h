/*
 * This file is part of fijuu2 (a.k.a fijuuu).
 * Copyright (C) 2006 Steven Pickles, Julian Oliver
 * 
 * fijuu2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * fijuu2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* huge slabs of this code are from the OGRE CubeMapping sample
 * thanks!  ;)
 *
 * -pix 
 */

#ifndef H_MeshWarp
#define H_MeshWarp

#define ENTITY_NAME "MeshWarppedEntity"
#define MESH_NAME "MeshWarppedMesh"

#define MATERIAL_NAME "Examples/SceneMeshWarp2"

/* ==================================================================== */
/*    Perlin Noise data and algorithms - copied from Perlin himself :)  */
/* ==================================================================== */
#define lerp(t,a,b) ( (a)+(t)*((b)-(a)) )
#define fade(t) ( (t)*(t)*(t)*(t)*((t)*((t)*6-15)+10) )
double grad(int hash, double x, double y, double z) {
	int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
	double u = h<8||h==12||h==13 ? x : y,   // INTO 12 GRADIENT DIRECTIONS.
		v = h<4||h==12||h==13 ? y : z;
	return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
int p[512]={
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,

	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

double noise3(double x, double y, double z) {
	int X = ((int)floor(x)) & 255,                  // FIND UNIT CUBE THAT
		Y = ((int)floor(y)) & 255,                  // CONTAINS POINT.
		Z = ((int)floor(z)) & 255;
	x -= floor(x);                                // FIND RELATIVE X,Y,Z
	y -= floor(y);                                // OF POINT IN CUBE.
	z -= floor(z);
	double u = fade(x),                                // COMPUTE FADE CURVES
		v = fade(y),                                // FOR EACH OF X,Y,Z.
		w = fade(z);
	int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
		B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

	return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
						grad(p[BA  ], x-1, y  , z   )), // BLENDED
					lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
						grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
				lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
						grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
					lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
						grad(p[BB+1], x-1, y-1, z-1 ))));
}


/* ==================================================================== */
/*                                 Main part                            */
/* ==================================================================== */

#define MAX_TWISTS 2

struct Twist
{

        Twist(Vector3 vector, Real gradient, Real offset)
        {
            mVector=vector;
            mVector.normalise();
            mAngleGradient=gradient;
            mAngleOffset=offset;
        }

        Twist() {
            Twist(Vector3::UNIT_Y,0,0);
        }

        Vector3 mVector;
        Radian mAngleGradient;
        Radian mAngleOffset;
};

class MeshWarpListener : public FrameListener
{
private:
	// main variables
	SceneManager *mSceneMgr ;
	SceneNode *objectNode ;

	// mesh-specific data
	MeshPtr originalMesh ;
	MeshPtr clonedMesh ;

	Entity *objectEntity ;

	// configuration
	Real displacement ;

        Twist twists[MAX_TWISTS];
        

	
	float* _normalsGetCleared(VertexData *vertexData)
	{
		const VertexElement *normVE = vertexData->
			vertexDeclaration->findElementBySemantic(VES_NORMAL);
		HardwareVertexBufferSharedPtr normHVB = vertexData->
			vertexBufferBinding->getBuffer(normVE->getSource());
		float* normals = (float*) normHVB->lock(0, normHVB->getSizeInBytes(), 
			HardwareBuffer::HBL_DISCARD);
		memset(normals, 0, normHVB->getSizeInBytes());
		return normals;
	}
	
	void _normalsSaveNormalized(VertexData *vertexData, float *normals)
	{
		const VertexElement *normVE = vertexData->
			vertexDeclaration->findElementBySemantic(VES_NORMAL);
		HardwareVertexBufferSharedPtr normHVB = vertexData->
			vertexBufferBinding->getBuffer(normVE->getSource());
		size_t numVertices = normHVB->getNumVertices();
		for(size_t i=0;i<numVertices;i++, normals+=3) {
			Vector3 n(normals[0], normals[1], normals[2]);
			n.normalise();
			normals[0] = n.x ;
			normals[1] = n.y ;
			normals[2] = n.z ;
		}
		normHVB->unlock();
	}
	
	void _updateVertexDataNoiseAndNormals(
			VertexData *dstData, 
			VertexData *orgData,
			IndexData *indexData,
			float *normals)
	{
		size_t i ;
		
		// Find destination vertex buffer
		const VertexElement *dstVEPos = dstData->
			vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr dstHVBPos = dstData->
			vertexBufferBinding->getBuffer(dstVEPos->getSource());
		// Find source vertex buffer 
		const VertexElement *orgVEPos = orgData->
			vertexDeclaration->findElementBySemantic(VES_POSITION);
		HardwareVertexBufferSharedPtr orgHVBPos = orgData->
			vertexBufferBinding->getBuffer(orgVEPos->getSource());
		// Lock these buffers
		float *dstDataPos = (float*) dstHVBPos->lock(0, dstHVBPos->getSizeInBytes(),
			HardwareBuffer::HBL_DISCARD);
		float *orgDataPos = (float*) orgHVBPos->lock(0, orgHVBPos->getSizeInBytes(),
			HardwareBuffer::HBL_READ_ONLY);
		// make noise
		size_t numVertices = orgHVBPos->getNumVertices();
		for(i=0;i<3*numVertices;i+=3) {
                    // NOTE: this is where the meshwarp should happen

                    Vector3 vert = Vector3(orgDataPos[i+0],orgDataPos[i+1],orgDataPos[i+2]);

                    
                    Matrix3 rotMat;
                        
                    // y = mx+c
                    for(int t=0;t<MAX_TWISTS;t++)
                    {
                        Twist &twist = twists[t];
                            Radian phi = (twist.mAngleGradient*twist.mVector.dotProduct(vert))+twist.mAngleOffset;

                            rotMat.FromAxisAngle(twist.mVector,phi);

                            vert=vert*rotMat;
                            
                    }

                    
                    // this is different from just adding noise to the length of the position vector
                    // TODO: would be great to add noise along the normal, but i'm not sure we can see them from here easily.
                    Vector3 disNoiseVert = Vector3(
                            Math::RangeRandom(-displacement,displacement),
                            Math::RangeRandom(-displacement,displacement),
                            Math::RangeRandom(-displacement,displacement));
                    
                    vert+=disNoiseVert;
                    //vert+=conNoiseVert*1.0;
                    
                    dstDataPos[i+0] = vert.x;
                    dstDataPos[i+1] = vert.y;
                    dstDataPos[i+2] = vert.z;

		}
		// Unlock original position buffer
		orgHVBPos->unlock();

		// calculate normals
		HardwareIndexBufferSharedPtr indexHB = indexData->indexBuffer ;
		unsigned short * vertexIndices = (unsigned short*) indexHB->lock(
			0, indexHB->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
		size_t numFaces = indexData->indexCount / 3 ;
		for(i=0 ; i<numFaces ; i++, vertexIndices+=3) {
			//~ int p0 = 0;
			//~ int p1 = 1;
			//~ int p2 = 2;
			int p0 = vertexIndices[0] ;
			int p1 = vertexIndices[1] ;
			int p2 = vertexIndices[2] ;

			//~ Vector3 v0(10,0,20);
			//~ Vector3 v1(30,0,20);
			//~ Vector3 v2(20,-1,50);
			Vector3 v0(dstDataPos[3*p0], dstDataPos[3*p0+1], dstDataPos[3*p0+2]);
			Vector3 v1(dstDataPos[3*p1], dstDataPos[3*p1+1], dstDataPos[3*p1+2]);
			Vector3 v2(dstDataPos[3*p2], dstDataPos[3*p2+1], dstDataPos[3*p2+2]);

			Vector3 diff1 = v1 - v2 ;
			Vector3 diff2 = v1 - v0 ;
			Vector3 fn = diff1.crossProduct(diff2);
#define _ADD_VECTOR_TO_REALS(ptr,vec) { *(ptr)+=vec.x; *((ptr)+1)+=vec.y; *((ptr)+2)+=vec.z; }
			_ADD_VECTOR_TO_REALS(normals+3*p0, fn);
			_ADD_VECTOR_TO_REALS(normals+3*p1, fn);
			_ADD_VECTOR_TO_REALS(normals+3*p2, fn);
#undef _ADD_VECTOR_TO_REALS
		}
		indexHB->unlock();

		// Unlock destination position buffer
		dstHVBPos->unlock();
	}

	void updateNoise()
	{
		float *sharedNormals = 0 ;
		for(int m=0;m<clonedMesh->getNumSubMeshes();m++) { // for each subMesh
			SubMesh *subMesh = clonedMesh->getSubMesh(m);
			SubMesh *orgSubMesh = originalMesh->getSubMesh(m);
			if (subMesh->useSharedVertices) {
				if (!sharedNormals) { // first of shared
					sharedNormals = _normalsGetCleared(clonedMesh->sharedVertexData);
				}
				_updateVertexDataNoiseAndNormals(
					clonedMesh->sharedVertexData, 
					originalMesh->sharedVertexData,
					subMesh->indexData,
					sharedNormals);
			} else {
				float* normals = _normalsGetCleared(subMesh->vertexData);
				_updateVertexDataNoiseAndNormals(
					subMesh->vertexData, 
					orgSubMesh->vertexData,
					subMesh->indexData,
					normals);
				_normalsSaveNormalized(subMesh->vertexData, normals);
			}
		}
		if (sharedNormals) {
			_normalsSaveNormalized(clonedMesh->sharedVertexData, sharedNormals);
		}
	}

	void clearEntity()
	{

		// detach and destroy entity
		objectNode->detachAllObjects();
		mSceneMgr->destroyEntity(ENTITY_NAME);
		
		// destroy mesh as well, to reset its geometry
		MeshManager::getSingleton().remove(clonedMesh->getHandle());
		
		objectEntity = 0 ;
	}
	
	VertexData* _prepareVertexData(VertexData *orgVD)
	{
		if (!orgVD)
			return 0 ;

        // Hacky bit: reorganise vertex buffers to a buffer-per-element
        // Since this demo was written a while back to assume that
        // Really this demo should be replaced with a vertex program noise
        // distortion, but left the software for now since it's nice for older
        // card owners
        VertexDeclaration* newDecl = orgVD->vertexDeclaration->clone();
        const VertexDeclaration::VertexElementList& elems = newDecl->getElements();
        VertexDeclaration::VertexElementList::const_iterator di;
        unsigned short buf = 0;
        for (di = elems.begin(); di != elems.end(); ++di)
        {
            newDecl->modifyElement(buf, buf, 0, di->getType(), di->getSemantic(), di->getIndex()); 
            buf++;
        }
        orgVD->reorganiseBuffers(newDecl);


		VertexData* newVD = new VertexData();
		// copy things that do not change
		newVD->vertexCount = orgVD->vertexCount ;
		newVD->vertexStart = orgVD->vertexStart ;
		// now copy vertex buffers, looking in the declarations
		VertexDeclaration* newVDecl = newVD->vertexDeclaration ;
		VertexBufferBinding* newVBind = newVD->vertexBufferBinding ;
		// note: I assume various semantics are not shared among buffers
		const VertexDeclaration::VertexElementList& orgVEL = orgVD->vertexDeclaration->getElements() ;
		VertexDeclaration::VertexElementList::const_iterator veli, velend;
		velend = orgVEL.end();
		// For each declaration, prepare buffer
		for( veli = orgVEL.begin() ; veli != velend ; ++veli)
		{
			VertexElementSemantic ves = (*veli).getSemantic();
			int source = (*veli).getSource() ;
			HardwareVertexBufferSharedPtr orgBuf = orgVD->vertexBufferBinding->
				getBuffer( source );
			// check usage for the new buffer
			bool dynamic = false ;
			switch(ves) {
				case VES_NORMAL :
				case VES_POSITION :
					dynamic = true ;
					break ;
				case VES_BLEND_INDICES : 
				case VES_BLEND_WEIGHTS :
				case VES_DIFFUSE : 
				case VES_SPECULAR :
				case VES_TEXTURE_COORDINATES :
				default :
					dynamic = false ;
					break ;
			}
			if (dynamic) { // create a new dynamic buffer with write access
				HardwareVertexBufferSharedPtr newBuf = 
					HardwareBufferManager::getSingleton().createVertexBuffer(
						orgBuf->getVertexSize(), orgBuf->getNumVertices(),
						HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE, 
						//~ HardwareBuffer::HBU_DYNAMIC, 
						true
						//~ false
						);
				newBuf->copyData(*orgBuf, 0, 0, orgBuf->getSizeInBytes(), true);
				newVBind->setBinding( source, newBuf );
			} else { // use the old one
				newVBind->setBinding( source, orgBuf );
			}
			// add element for declaration
			newVDecl->addElement(source, (*veli).getOffset(), (*veli).getType(),
				ves, (*veli).getIndex());
		}

		return newVD;
	}
	
	void prepareClonedMesh()
	{
		// we create new Mesh based on the original one, but changing
		// HBU flags (inside _prepareVertexData)
		clonedMesh = MeshManager::getSingleton().createManual(MESH_NAME, 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		clonedMesh->_setBounds(originalMesh->getBounds()); 
		clonedMesh->_setBoundingSphereRadius(originalMesh->getBoundingSphereRadius());
		//~ if (originalMesh->sharedVertexData)
			//~ clonedMesh->sharedVertexData = originalMesh->sharedVertexData->clone();
		clonedMesh->sharedVertexData = 
			_prepareVertexData(originalMesh->sharedVertexData);
		for(int sm=0;sm<originalMesh->getNumSubMeshes();sm++) {
			SubMesh *orgSM = originalMesh->getSubMesh(sm);
			SubMesh *newSM = clonedMesh->createSubMesh();
			if (orgSM->isMatInitialised()) {
				newSM->setMaterialName(orgSM->getMaterialName());
			}
			newSM->useSharedVertices = orgSM->useSharedVertices ;
			// prepare vertex data
			newSM->vertexData = _prepareVertexData(orgSM->vertexData);
			// reuse index data
			newSM->indexData->indexBuffer = orgSM->indexData->indexBuffer ;
			newSM->indexData->indexStart = orgSM->indexData->indexStart ;
			newSM->indexData->indexCount = orgSM->indexData->indexCount ;
		}
	}

	void prepareEntity(const String& meshName) 
	{
		if (objectEntity) {
			clearEntity();
		}
		
		// load mesh if necessary - note, I assume this is the only point
		// Mesh can get loaded, since I want to make sure about its HBU etc.
		originalMesh = MeshManager::getSingleton().getByName(meshName);
		if (originalMesh.isNull()) {
			originalMesh = MeshManager::getSingleton().load(meshName,
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
				true, true); //so we can still read it
			if (originalMesh.isNull()) {
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
					"Can't find a mesh: '"+meshName+"'",
					"MeshWarpListener::prepareEntity");
			}
		}
		
		
		prepareClonedMesh();

        // create an entity based on cloned mesh
		objectEntity = mSceneMgr->createEntity( ENTITY_NAME, MESH_NAME);
		objectNode->attachObject(objectEntity);
		
		// update noise to avoid one frame w/o noise
		//if (noiseOn)
                updateNoise();
	}



public:

        Entity* getObjectEntity()
        {
            return objectEntity;
        }
        
        void setObject(String meshName)
        {
		printf("Switching to object: %s\n", meshName.c_str());
		prepareEntity(meshName);
	}

        void setDisplacement(Real displacement)
        {
            this->displacement = displacement;
        }

      
        void setTwist(unsigned int n, Vector3 vector, Real gradient, Real offset)
        {
            if(n<MAX_TWISTS) {
                Twist &twist = twists[n];
                twist = Twist(vector,gradient,offset);
            }
            else
            {
                LogManager::getSingleton().logMessage("Twist index "+StringConverter::toString(n)+" is out of range, ignored.");
            }
        }
        
        
    MeshWarpListener(/*RenderWindow* win, Camera* cam,*/
			SceneManager *sceneMgr, SceneNode *objectNode)
    {
		this->mSceneMgr = sceneMgr ;
		this->objectNode = objectNode ;

		displacement = 0.0f;
		objectEntity = 0 ;

                for(int i=0;i<MAX_TWISTS;i++)
                    setTwist(i,Vector3::UNIT_Y,0,0);
    }
    virtual bool frameStarted(const FrameEvent& evt)
    {
			updateNoise();

        
                        return true;
    }

} ;

#endif
