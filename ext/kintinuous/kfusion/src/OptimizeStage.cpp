#include <kfusion/OptimizeStage.hpp>

// default constructor
OptimizeStage::OptimizeStage() : AbstractStage()
	,mesh_count_(0),textured(true),bounding_counter(0),texture_counter(0),pic_count_(0)
{
	timestamp.setQuiet(true);
}

void OptimizeStage::firstStep() { optiMesh_ = NULL; }

void OptimizeStage::step()
{
	auto mesh_work = boost::any_cast<pair<pair<MeshPtr, bool>, vector<kfusion::ImgPose*> > >(getInQueue()->Take());
	bool last_shift = mesh_work.first.second;
	MeshPtr act_mesh = mesh_work.first.first;
	string mesh_notice = ("#### C:            Mesh Optimization " +  to_string(mesh_count_) + "    ####");
	ScopeTime* opti_time = new ScopeTime(mesh_notice.c_str());
	ScopeTime* opo_time = new ScopeTime("All without retesselation");
	if(optiMesh_ == NULL)
		optiMesh_ = act_mesh;
	else
		optiMesh_->addMesh(act_mesh);
	std::vector<kfusion::ImgPose*> image_poses_buffer = mesh_work.second;
	std::cout << "Loaded " << image_poses_buffer.size() << " Images. " << std::endl;
	
	optiMesh_->optimizePlanes(3, 0.85, 7, 0);
	
	//act_mesh->optimizePlaneIntersections();
	std::cout << "start_texture_index: " << texture_counter << std::endl;
	MeshPtr tmp_pointer = optiMesh_->retesselateInHalfEdge(0.01,textured,texture_counter);
	if(!last_shift)
		optiMesh_->deleteRegions();
	if(tmp_pointer  == NULL)
	{
		cout <<"skipped " << endl;
		return;
	}
	std::cout << "            ####     3 Finished optimisation number: " << mesh_count_ << "   ####" << std::endl;
	mesh_count_++;
	///texturing
	if(textured){
		int counter=0;
		int i;
		for(i=0;i<image_poses_buffer.size();i++){
			counter = tmp_pointer->projectAndMapNewImage(*(image_poses_buffer[i]));
		}
		pic_count_+=i;
		
		
		
		texture_counter += tmp_pointer->textures.size();
		
		meshBufferPtr = tmp_pointer->meshBuffer();
				
	}
	image_poses_buffer.resize(0);
	
	
	getOutQueue()->Add(pair<pair<MeshPtr, bool>, vector<kfusion::ImgPose*> >(
				pair<MeshPtr, bool>(tmp_pointer, last_shift), mesh_work.second));
	//delete act_mesh;
	if(last_shift)
		done(true);
}
void OptimizeStage::lastStep()	{ 
	
	if(textured)
	{
		int i=0;
		//PointSave saver;
		//saver.saveBoundingRectangles("b_rects.ply",global_tmp_pointer->getBoundingRectangles(i),global_tmp_pointer->getBoundingRectangles(i).size()*4);
		//cout << i/4 << " Bounding Rectangles" << endl;
	
		ModelPtr dst_model(new Model(meshBufferPtr));
		ModelFactory::saveModel(dst_model, "./mesh_OUT.obj");
		cout << "File saved to mesh_OUT.obj" << endl;
	}
	
};

