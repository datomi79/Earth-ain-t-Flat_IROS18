#include <ceres/ceres.h>
#include <ceres/rotation.h>


// Read a problem file specified in 'my' format
class SingleViewPoseAdjustmentProblem{

public:

	// Return the number of keypoints observed
	int getNumPts() const { return numPts_; }
	// Return the center of the car
	double* getCarCenter() { return carCenter_; }
	// Return the height of the car
	double getCarHeight() const { return h_; }
	// Return the width of the car
	double getCarWidth() const { return w_; }
	// Return the length of the car
	double getCarLength() const { return l_; }
	// Return a pointer to the observation vector
	double* observations() const { return observations_; }
	// Return a pointer to the observation weights vector
	double* observationWeights() const { return observationWeights_; }
	// Return a pointer to the camera intrinsics
	double* getK() const { return K_; }
	// Return a pointer to the mean 3D locations
	double* getX_bar() const { return X_bar_; }
	// Return the number of eigen vectors
	int getNumVec() const { return numVec_; }
	// Return a pointer to the top 5 eigenvectors
	double* getV() { return V_; }
	// Return a pointer to the weights (lambdas)
	double* getLambdas() { return lambdas_; }

	// Read data from input file
	bool loadFile(const char *fileName){
		FILE *fptr = fopen(fileName, "r");
		if(fptr == NULL){
			return false;
		}

		// numPts
		fscanfOrDie(fptr, "%d", &numPts_);
		

		// Center of the car
		carCenter_ = new double[3];
		fscanfOrDie(fptr, "%lf", carCenter_+0);
		fscanfOrDie(fptr, "%lf", carCenter_+1);
		fscanfOrDie(fptr, "%lf", carCenter_+2);

		// Height, Width, and Length of the Car
		fscanfOrDie(fptr, "%lf", &h_);
		fscanfOrDie(fptr, "%lf", &w_);
		fscanfOrDie(fptr, "%lf", &l_);

		// Camera intrinsics
		K_ = new double[9];
		for(int i = 0; i < 9; ++i){
			fscanfOrDie(fptr, "%lf", K_ + i);
		}

		// Observations
		observations_ = new double[2*numPts_];
		for(int i = 0; i < numPts_; ++i){
			for(int j = 0; j < 2; ++j){
				fscanfOrDie(fptr, "%lf", observations_ + i*2 + j);
			}
		}

		// Observation weights
		observationWeights_ = new double[numPts_];
		for(int i = 0; i < numPts_; ++i){
			fscanfOrDie(fptr, "%lf", observationWeights_ + i);
		}

		// Mean locations
		X_bar_ = new double[3*numPts_];
		for(int i = 0; i < numPts_; ++i){
			for(int j = 0; j < 3; ++j){
				fscanfOrDie(fptr, "%lf", X_bar_ + i*3 + j);
				//std::cout<<*(X_bar_ + i*3 + j)<<" ";
			}
			//std::cout<<"\n";
		}

		fscanfOrDie(fptr, "%d", &numVec_);

		// Read in the top 42 eigenvectors for the shape
		// Size allocation: 42 vecs * 3 coordinates per vex * 36 keypoints (numPts_)
		V_ = new double[numVec_*3*numPts_];
		for(int i = 0; i < numVec_; ++i){
			for(int j = 0; j < numPts_; ++j){
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 0);
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 1);
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 2);
			}
		}

		// Read in the initial values for lambdas
		lambdas_ = new double[numVec_];
		for(int i = 0; i < numVec_; ++i){
			fscanfOrDie(fptr, "%lf", lambdas_ + i);
		}
		
		return true;

	}

private:

	// Helper function to read in one value to a text file
	template <typename T>
	void fscanfOrDie(FILE *fptr, const char *format, T *value){
		int numScanned = fscanf(fptr, format, value);
		if(numScanned != 1){
			LOG(FATAL) << "Invalid data file";
		}
	}

	// Private variables
	
	// Number of observations
	int numPts_;

	// Center of the car
	double *carCenter_;
	// Dimensions of the car
	double h_, w_, l_;

	// Camera intrinsics
	double *K_;
	// Observation vector
	double *observations_;
	// Observation weight vector
	double *observationWeights_;
	// 3D point
	double *X_bar_;
	// Number of eigen vectors.
	int numVec_;

	// Top numVec eigenvectors for the shape, i.e., the deformation basis vectors
	double *V_;

	// Weights for the eigenvectors
	double *lambdas_;

	// Rough center of the car
	double *carCenter3D_;

};



class GroundPlaneAdjustmentProblem{


public:
	double getNumViews() { return numViews_;}
	double getNumPts() { return numPoints_;}
	double *get3DPts() { return Xs_; }
	double *get2DPts() { return xs_; }
	double *getK() { return K_;}
	double *getRs() { return Rs_;}
	double *getts() { return ts_;}
	double *getPlaneParameters() { return plane_;}

	bool loadFile(const char *fileName){

		
		FILE *fptr = fopen(fileName, "r");
		if(fptr == NULL){
			return false;
		}

		// Read mumber of views and points.
		fscanfOrDie(fptr,"%d",&numViews_);
		fscanfOrDie(fptr,"%d",&numPoints_);


		// Read K matrix
		K_ = new double[9];
		for(int i=0;i<9;++i){
			fscanfOrDie(fptr,"%lf",K_ + i);
            //std::cout<<K_[0]<<" "<<K_[1]<<" "<<K_[2]<<" "<<K_[3]<<" "<<K_[4]<<" "<<K_[5]<<" "<<K_[6]<<" "<<K_[7]<<" "<<K_[8]<< "\n";	
		}
	
		//Read 3D points : rougly initilaized via triangulation.
		Xs_ = new double[3*numPoints_];
		for(int i=0;i<numPoints_;i++){
			for(int j=0;j<3;j++)
				fscanfOrDie(fptr,"%lf",Xs_ + 3*i + j);			
			//std::cout<<Xs_[3*i + 0]<<" "<<Xs_[3*i + 1]<<" "<<Xs_[3*i+2]<< "\n";	
		}

		
		// Read R and t
		Rs_=new double[9*numViews_];
		 for(int i=0;i<numViews_;i++){
		 	for(int j=0;j<9;j++)
		 		fscanfOrDie(fptr,"%lf",Rs_ + 9*i + j);
		}

		ts_=new double[3*numViews_];
		for(int i=0;i<numViews_;i++){
			for(int j=0;j<3;j++)
				fscanfOrDie(fptr,"%lf",ts_ + 3*i + j);
		}

		// Read 2D points
		xs_ =new double[2*numPoints_*numViews_];
		for(int i=0;i<numViews_;i++){
			for(int j=0;j<numPoints_;j++)
				for(int k=0;k<2;k++)
					fscanfOrDie(fptr,"%lf",xs_ + i*2*numPoints_ + 2*j + k);
		}

		/*for(int i=3;i<4;i++){
			for(int j=0;j<numPoints_;j++)
					std::cout<<xs_[numPoints_*2*i + 2*j +0] <<" "<<xs_[numPoints_*2*i +2*j +1]<<"\n";
		}*/
		
		// Read the ground plane parameters
		plane_ = new double[4];
		for(int i=0;i<4;i++){
			fscanfOrDie(fptr,"%lf",plane_ + i);
		}	

		

		// TO CHECK :


		/*std::cout << numViews_ << " " << numPoints_ << std::endl;
		for(int i=0;i<numPoints_;i++){
			std::cout << Xs_[3*i+0 ] << " " << Xs_[3*i+1] << " " << Xs_[3*i+2] << std::endl;
		}*/

		/*
		for(int i=0;i<numViews_;i++){
			std::cout << Rs_[9*i+0 ] << " " << Rs_[9*i+1] << " " << Rs_[9*i+2] <<" " << Rs_[9*i+3]<<" " << Rs_[9*i+4]<<" " << Rs_[9*i+5]<<" " << Rs_[9*i+6]<<" " << Rs_[9*i+7]<<" " << Rs_[9*i+8]<< std::endl;

		}
		*/

		/*
		for(int i=0;i<numViews_;i++){
			std::cout << ts_[3*i+0 ] << " " << ts_[3*i+1] << " " <<ts_[3*i+2] << std::endl;

		}
		*/
		
		/*for(int i=0;i<numViews_;i++){
			for(int j=0;j<numPoints_;j++){
				for(int k=0;k<2;k++){
					std::cout << xs_[i*2*numPoints_ + 2*j + k] << " ";
				}
				std::cout<<std::endl;
			}
			std::cout<<std::endl;
		}
		*/

		//std::cout << plane_[0] << " " << plane_[1] <<" " <<  " " << plane_[2] << " " <<plane_[3] << std::endl;

		return true;

		
		

	}

private:

	// Helper function to read in one value to a text file
	template <typename T>
	void fscanfOrDie(FILE *fptr, const char *format, T *value){
		int numScanned = fscanf(fptr, format, value);
		if(numScanned != 1){
			LOG(FATAL) << "Invalid data file";
		}
	}

	// Num of views
	int numViews_;
	// Num of points
	int numPoints_;
	// Variable that stores the K ,camera intrinsics
	double *K_;
	// Variable that stores the rotation and translation
	double *Rs_;
	double *ts_;
	// Storing 3D and 2D points.
	double *Xs_;
	double *xs_;
	// Reading the ground plane parameters
	double *plane_;
};



// Read a shape adjustment problem (a single view one)
class SingleViewShapeAdjustmentProblem{

public:

	// Return the number of keypoints observed
	int getNumPts() const { return numPts_; }
	// Return the center of the car
	double* getCarCenter() { return carCenter_; }
	// Return the height of the car
	double getCarHeight() const { return h_; }
	// Return the width of the car
	double getCarWidth() const { return w_; }
	// Return the length of the car
	double getCarLength() const { return l_; }
	// Return a pointer to the observation vector
	double* observations() const { return observations_; }
	// Return a pointer to the observation weights vector
	double* observationWeights() const { return observationWeights_; }
	// Return a pointer to the camera intrinsics
	double* getK() const { return K_; }
	// Return a pointer to the mean 3D locations
	double* getX_bar() const { return X_bar_; }
	// Return the number of eigen vectors
	int getNumVec() const { return numVec_; }
	// Return a pointer to the top 5 eigenvectors
	double* getV() { return V_; }
	// Return a pointer to the weights (lambdas)
	double* getLambdas() { return lambdas_; }
	// Return a pointer to the rotation estimated (from PnP)
	double* getRot() { return rot_; }
	// Return a pointer to the translation estimated (from PnP)
	double* getTrans() { return trans_; }

	// Read data from input file
	bool loadFile(const char *fileName){
		FILE *fptr = fopen(fileName, "r");
		if(fptr == NULL){
			return false;
		}

		
		fscanfOrDie(fptr, "%d", &numPts_);

		// Center of the car
		carCenter_ = new double[3];
		fscanfOrDie(fptr, "%lf", carCenter_+0);
		fscanfOrDie(fptr, "%lf", carCenter_+1);
		fscanfOrDie(fptr, "%lf", carCenter_+2);

		// Height, Width, and Length of the Car
		fscanfOrDie(fptr, "%lf", &h_);
		fscanfOrDie(fptr, "%lf", &w_);
		fscanfOrDie(fptr, "%lf", &l_);

		// Camera intrinsics
		K_ = new double[9];
		for(int i = 0; i < 9; ++i){
			fscanfOrDie(fptr, "%lf", K_ + i);
		}
		
		// Observations
		observations_ = new double[2*numPts_];
		for(int i = 0; i < numPts_; ++i){
			for(int j = 0; j < 2; ++j){
				fscanfOrDie(fptr, "%lf", observations_ + i*2 + j);
			}
		}

		// Observation weights
		observationWeights_ = new double[numPts_];
		for(int i = 0; i < numPts_; ++i){
			fscanfOrDie(fptr, "%lf", observationWeights_ + i);
		}

		// Mean locations
		X_bar_ = new double[3*numPts_];
		for(int i = 0; i < numPts_; ++i){
			for(int j = 0; j < 3; ++j){
				fscanfOrDie(fptr, "%lf", X_bar_ + i*3 + j);
			}
		}

		fscanfOrDie(fptr, "%d", &numVec_);

		// Read in the top 5 eigenvectors for the shape
		// Size allocation: 5 vecs * 3 coordinates per vex * 14 keypoints (numPts_)
		V_ = new double[numVec_*3*numPts_];
		for(int i = 0; i < numVec_; ++i){
			for(int j = 0; j < numPts_; ++j){
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 0);
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 1);
				fscanfOrDie(fptr, "%lf", V_ + i*3*numPts_ + 3*j + 2);
			}
		}

		// Read in the initial values for lambdas
		lambdas_ = new double[numVec_];
		for(int i = 0; i < numVec_; ++i){
			fscanfOrDie(fptr, "%lf", lambdas_ + i);
		}

		// Read in the rotation estimate (from PnP) (column-major ordered rotation matrix)
		rot_ = new double[9];
		for(int i = 0; i < 9; ++i){
			fscanfOrDie(fptr, "%lf", rot_ + i);
		}

		// Read in the translation estimate (from PnP)
		trans_ = new double[3];
		for(int i = 0; i < 3; ++i){
			fscanfOrDie(fptr, "%lf", trans_ + i);
		}

		return true;

	}

private:

	// Helper function to read in one value to a text file
	template <typename T>
	void fscanfOrDie(FILE *fptr, const char *format, T *value){
		int numScanned = fscanf(fptr, format, value);
		if(numScanned != 1){
			LOG(FATAL) << "Invalid data file";
		}
	}

	// Private variables
	
	// Number of observations
	int numPts_;

	// Center of the car
	double *carCenter_;
	// Dimensions of the car
	double h_, w_, l_;

	// Camera intrinsics
	double *K_;
	// Observation vector
	double *observations_;
	// Observation weight vector
	double *observationWeights_;
	// 3D point
	double *X_bar_;
	// The number of eigen vectors
	int numVec_;

	// Top numVec eigenvectors for the shape, i.e., the deformation basis vectors
	double *V_;

	// Weights for the eigenvectors
	double *lambdas_;

	// Rotation estimate (from PnP)
	double *rot_;
	// Translation estimate (from PnP)
	double *trans_;

};

// Read a problem file specified in 'my' format
class MultiViewShapeandPoseAdjuster{

public:

	// Return number of views.
	int getNumViews() const { return numViews_;}
	// Return the number of observations
	int getNumObs() const { return numObs_; }
	// Return the number of keypoints observed
	int getNumPts() const { return numPts_; }
	// Return the center of the car
	double* getCarCenter() { return carCenter_; }
	// Return the height of the car
	double getCarHeight() const { return h_; }
	// Return the width of the car
	double getCarWidth() const { return w_; }
	// Return the length of the car
	double getCarLength() const { return l_; }
	// Return a pointer to the observation vector
	double* observations() const { return observations_; }
	// Return a pointer to the observation weights vector
	double* observationWeights() const { return observationWeights_; }
	// Return a pointer to the camera intrinsics
	double* getK() const { return K_; }
	// Return a pointer to the mean 3D locations
	double* getX_bar() const { return X_bar_; }
	// Return the number of vectors
	int getNumVec() const { return numVec_;}
	// Return a pointer to the top 5 eigenvectors
	double* getV() { return V_; }
	// Return a pointer to the weights (lambdas)
	double* getLambdas() { return lambdas_; }
	// Return the rotations
	double* getRotations() { return rot_; }
	// Return the translations.
	double* getTranslations() { return trans_; }

	// Read data from input file
	bool loadFile(const char *fileName){

		FILE *fptr = fopen(fileName, "r");
		if(fptr == NULL){
			return false;
		}

		// numViews, numPts, numObs, numFaces
		fscanfOrDie(fptr, "%d", &numViews_);
		fscanfOrDie(fptr, "%d", &numPts_);
		fscanfOrDie(fptr, "%d", &numObs_);

		// Height, Width, and Length of the Car
		fscanfOrDie(fptr, "%lf", &h_);
		fscanfOrDie(fptr, "%lf", &w_);
		fscanfOrDie(fptr, "%lf", &l_);


		// Camera intrinsics
		K_ = new double[9];
		for(int i = 0; i < 9; ++i){
			fscanfOrDie(fptr, "%lf", K_ + i);
		}

		// For each view,scan the following : center of the car, keypoints positions,keypoints weights,shape and vector.

		// Center of the car
		carCenter_ = new double[numViews_*3];
		for(int i=0;i<numViews_;++i){
			fscanfOrDie(fptr, "%lf", carCenter_ + 3*i +0);
			fscanfOrDie(fptr, "%lf", carCenter_ + 3*i +1);
			fscanfOrDie(fptr, "%lf", carCenter_ + 3*i +2);
		}

		/*for(int i=0;i<numViews_;i++){
			std::cout << carCenter_[3*i + 0] << " " << carCenter_[3*i + 1] << " " << carCenter_[3*i + 2] << std::endl;
		}*/
		
		// Keypoint Observations
		observations_ = new double[numViews_*2*numObs_];
		for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; j++){
				for(int k = 0; k < 2; k++){
					fscanfOrDie(fptr, "%lf", observations_ + i*numObs_*2 + j*2 + k);
				}
			}
		}

		/*for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; j++){
					std::cout << observations_[i*numObs_*2 + j*2 +0] << " " << observations_[i*numObs_*2 + j*2+ 1] << std::endl;		
			}
		}*/

		// Observation weights
		observationWeights_ = new double[numViews_*numObs_];
		for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; ++j){
				fscanfOrDie(fptr, "%lf", observationWeights_ + i*numObs_ + j);
			}
		}
		/*for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; j++){
					std::cout << observationWeights_[i*numObs_ + j]<< std::endl;		
			}
		}*/


	

		// Mean locations
		X_bar_ = new double[numViews_*3*numObs_];
		for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; ++j){
				for(int k = 0; k < 3; ++k){
					fscanfOrDie(fptr, "%lf", X_bar_ + i*3*numObs_+ j*3 + k);
				}
			}
		}

		/*for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numObs_; ++j){
				std::cout << X_bar_[i*3*numObs_+ j*3 + 0] << " " << X_bar_[i*3*numObs_+ j*3 + 1] << " " << X_bar_[i*3*numObs_+ j*3 + 2] <<std::endl;
			}
		}*/

		// Read the number of eigen vectors
		fscanfOrDie(fptr, "%d", &numVec_);

		// Read in the top 5 eigenvectors for the shape
		// Size allocation: 5 vecs * 3 coordinates per vex * 14 keypoints (numPts_)
		V_ = new double[numViews_*numVec_*3*numPts_];
		for(int i=0;i<numViews_;i++){
			for(int j = 0; j < numVec_; ++j){
				for(int k = 0; k < numPts_; ++k){
					fscanfOrDie(fptr, "%lf", V_ + i*3*numVec_*numPts_ + j*3*numPts_ + 3*k + 0);
					fscanfOrDie(fptr, "%lf", V_ + i*3*numVec_*numPts_ + j*3*numPts_ + 3*k + 1);
					fscanfOrDie(fptr, "%lf", V_ + i*3*numVec_*numPts_ + j*3*numPts_ + 3*k + 2);
			
				}
			}
		}
		

		/*for(int i=0;i<1;i++){
			for(int j = 0; j < 5; ++j){
				for(int k = 0; k < numPts_; ++k){
					std::cout << V_[i*3*5*numPts_ + j*3*numPts_ + 3*k + 0] << " " << V_[i*3*5*numPts_ + j*3*numPts_ + 3*k + 1] << " " << V_[i*3*5*numPts_ + j*3*numPts_ + 3*k + 2] <<std::endl;
				}
			}
		}*/


		// Read in the initial values for lambdas
		lambdas_ = new double[numVec_];
		for(int i=0;i<numVec_;i++){
			fscanfOrDie(fptr, "%lf", lambdas_ + i);
		}




		// Read in the rotation estimate (from PnP) (column-major ordered rotation matrix)
		rot_ = new double[numViews_*9];
		for(int i = 0; i < numViews_; i++){
			for(int j=0;j<9;j++){
				fscanfOrDie(fptr, "%lf", rot_ + i*9 + j);
			}
		}
		
		// Read in the translation estimate (from PnP)
		trans_ = new double[numViews_*3];
		for(int i = 0; i < numViews_; i++){
			for(int j=0;j<3;j++){
				fscanfOrDie(fptr, "%lf", trans_ + i*3 + j);
			}
		}

		//std::cout << trans_[0] << " "  << trans_[1] <<" "  << trans_[2]<<std::endl;




		return true;


	}

private:

	// Helper function to read in one value to a text file
	template <typename T>
	void fscanfOrDie(FILE *fptr, const char *format, T *value){
		int numScanned = fscanf(fptr, format, value);
		if(numScanned != 1){
			LOG(FATAL) << "Invalid data file";
		}
	}

	// Private variables
	
	// Number of views
	int numViews_;
	// Number of keypoints
	int numPts_;
	// Number of observations
	int numObs_;

	// Center of the car
	double *carCenter_;
	// Dimensions of the car
	double h_, w_, l_;

	// Camera intrinsics
	double *K_;
	// Observation vector
	double *observations_;
	// Observation weight vector
	double *observationWeights_;
	// 3D point
	double *X_bar_;
	// Get the number of eigen vectors
	int numVec_;

	// Top eigenvectors for the shape, i.e., the deformation basis vectors
	double *V_;
	// Weights for the eigenvectors
	double *lambdas_;
	// Rotations in each frame.
	double *rot_;

	// Translation for each frame.
	double *trans_;

};
