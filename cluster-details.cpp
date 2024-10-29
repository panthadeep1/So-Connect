#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <cmath>
#include <cstring>
#include <cassert>
#include <string>
#include <queue>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <unistd.h>
#include <ios>
#include <sstream>
#include <queue>
#include <sys/time.h>
using namespace std;

typedef struct Clust{

     public: 
      int status;
      int rep;
      int core;
      
}Cluster;

typedef struct kLayers{

     public: 
      int layer_Num;
      int point_Num;
      double edge_Sim;
      
}Layer;

bool myComp(const Layer &a, const Layer &b) {
  return (a.edge_Sim > b.edge_Sim);
}

//Required global Data-structures
vector< vector<double> >SNG_matrix; 
vector< vector< pair<int,double> > >KSNG_matrix;

vector<Cluster>pCluster; //storing the "pts' Cluster" status from <SDB_pred>
vector< vector<int> >gCluster; //storing "graph of Clusters" in a 2-D vector

vector< vector< Layer > >pLayer; //storing "Layers of each point" wrt. pair<Layer id,similarity>
vector< vector< Layer > >unReco; //Unrecommended connections

//Core, non-core, noise pts.
vector<int>core_cluster;
vector<int>non_core_cluster;
vector<int>non_core_noise;


//Global variables
double SNN, delta_link; //SNN
int nrow, ncol;
int core, non_core, noise;





//START___main()___
int main(int argc, char* argv[])
{
	  string input_file = argv[1];
	  string input_file_bin = input_file+".bin";
	  string input_file_string = argv[1];	  
	  
	  string str, word;
	  ifstream in("Row_Column", ios::in);
	  getline(in,str);
          stringstream ss(str);
          ss>>nrow>>ncol;
          
          cout<<"#Rows:"<<nrow<<endl;
          cout<<"#Columns:"<<ncol<<endl;
	  
	                                                     
					                             //***Load the SNN value***//
					                             
				  double t1_load_SNN=(double)clock()/CLOCKS_PER_SEC;
				  
				  ifstream in_SNN("SNN.bin",ios::in|ios::binary);
				  if(!in_SNN)
				  {cout<<"SNN.bin file is not found check again!!"<<endl; exit(0);}
				  cout<<"Loaded SNN file: SNN.bin"<<endl;   
				  
				  //Read the nrow,ncol values from input binary file
	    			  in_SNN.read((char*)&SNN,sizeof(double)); 
	    			  delta_link = SNN; 
	    			  in_SNN.close();
	    			  
	    		          double t2_load_SNN=(double)clock()/CLOCKS_PER_SEC;
	    			  cout<<"Value of SNN(delta_link) = "<<delta_link<<": "<<(t2_load_SNN-t1_load_SNN)<<" s"<<endl;   
		    			  
		    			  
		    			  
		    			                            //***Load the SNG matrix***//    
		    		
		    		  double t1_load_SNG=(double)clock()/CLOCKS_PER_SEC;	
		    		  	    		  	               
	    			  ifstream in_SNG("SNG_matrix.bin",ios::in|ios::binary);
				  if(!in_SNG)
				  {cout<<"SNG_matrix.bin file is not found check again!!"<<endl; exit(0);}
				  cout<<"Loaded SNG_matrix file: SNG_matrix.bin"<<endl;      
				  
				  SNG_matrix.resize(nrow);
				  double temp_SNG;
					 for(int i=0;i<nrow;i++)
					 {  
						 int row_size;
						    in_SNG.read((char*)&row_size,sizeof(row_size));
						    if(row_size == 0) continue; //No adjacent links

						    if(row_size > 0)
						    {
							      for(int j=0;j<row_size;j++)
							       {
								  in_SNG.read((char*)&temp_SNG,sizeof(temp_SNG));
								  SNG_matrix[i].push_back(temp_SNG);									  
							       }
						    }

					 }//end-FOR
				 in_SNG.close();   
				    
				 double t2_load_SNG=(double)clock()/CLOCKS_PER_SEC;
				 				 
				  cout<<"The SNG matrix is: "<<(t2_load_SNG-t1_load_SNG)<<" s"<<endl;
					    /*for(int i=0;i<nrow;i++)
					       {
						    cout<<"User #"<<i<<":\n";
							  for(int j=0;j<SNG_matrix[i].size();j++)
							  {
								     cout<<"("<<i<<","<<j<<"):";
								     cout<<"["<<SNG_matrix[i][j]<<"] ";
							  }
						       
							 cout<<endl<<"----------"<<endl;
					       }*/	
					       
					       
					       
					                             //***Load the KSNG matrix***// 
					                             
					                  
			          double t1_load_KSNG=(double)clock()/CLOCKS_PER_SEC;
			          		    		  	               
	    			  ifstream in_KSNG("KSNG_matrix.bin",ios::in|ios::binary);
				  if(!in_KSNG)
				  {cout<<"KSNG_matrix.bin file is not found check again!!"<<endl; exit(0);}
				  cout<<"Loaded KSNG_matrix file: KSNG_matrix.bin"<<endl;      
				  
				  KSNG_matrix.resize(nrow);
				  pair<int,double>temp_KSNG;
					 for(int i=0;i<nrow;i++)
					 {  
						 int row_size;
						    in_KSNG.read((char*)&row_size,sizeof(row_size));
						    if(row_size == 0) continue; //No adjacent links

						    if(row_size > 0)
						    {
							      for(int j=0;j<row_size;j++)
							       {
								  in_KSNG.read((char*)&temp_KSNG,sizeof(temp_KSNG));
								  KSNG_matrix[i].push_back(temp_KSNG);									  
							       }
						    }

					 }//end-FOR
				 in_KSNG.close();      
				 
				 double t2_load_KSNG=(double)clock()/CLOCKS_PER_SEC;
				 
				  cout<<"The KSNG matrix is: "<<(t2_load_KSNG-t1_load_KSNG)<<" s"<<endl;
					    /*for(int i=0;i<nrow;i++)
					       {
						    cout<<"User #"<<i<<" ("<<KSNG_matrix[i].size()<<" adjacent links)\n";
							  for(int j=0;j<KSNG_matrix[i].size();j++)
							  {
								cout<<"("<<i<<","<<j<<"):";
							        cout<<"["<<KSNG_matrix[i][j].first<<"/";
							        cout<<KSNG_matrix[i][j].second<<"] ";
							  }
						       
							 cout<<endl<<"----------"<<endl;
					       }*/	
                                                                                                                                        
                                                     	                                                     
                                                     	                                                     
                                                                       //*** Load the Clusters ***/ 
                                                                        
                                                                       
					  double t1_load_clust=(double)clock()/CLOCKS_PER_SEC;
					  
					  ifstream in_Clust(input_file_bin.c_str(),ios::in|ios::binary);
					  if(!in_Clust)
					  {cout<<input_file_bin<<" Cluster file is not found check again!!"<<endl; exit(0);}
					  cout<<"Loaded Cluster file: "<<input_file_bin<<endl;
					  
					  Cluster temp_cluster; //Declare the cluster struct variable					  
					  for(int i=0;i<nrow;i++)
					  {
					       in_Clust.read((char*)&temp_cluster,sizeof(temp_cluster));
					       pCluster.push_back(temp_cluster);
					  }
					  in_Clust.close();
					  
					  double t2_load_clust=(double)clock()/CLOCKS_PER_SEC;
					  
					  cout<<"The points' cluster list is: "<<(t2_load_clust-t1_load_clust)<<" s"<<endl;					   
					        /*for(int i=0;i<nrow;i++)
						 {
							   cout<<i<<"]";
							   cout<<pCluster[i].status<<"/";
							   cout<<pCluster[i].rep<<"/";
							   cout<<pCluster[i].core;
							   cout<<endl;

						 }*/						 
						 						 
							 
				 
							
							//***** START of the Code for LAYER-wise <SDB_pred> *****//
							
							
				double t1_gClust=(double)clock()/CLOCKS_PER_SEC;
							
				gCluster.resize(nrow);	//Creation of the vector graph clusters				
				core = non_core = noise = 0;			
				for(int i=0;i<nrow;i++)
				{
				      int rept = pCluster[i].rep; //store the cluster representative pt.
				      int point = i;
				    
					    if(rept == -1) //noise pt.
					    {
					       non_core_noise.push_back(point);
					       noise++;
					       continue;
					    }   
					    
					    if(rept >=0) //rep. point index >=0
					    {
						       if(pCluster[i].core == 0) //non-core pt.
						       {
							  non_core_cluster.push_back(point);
							  non_core++; 
						       }
						       
						       if(pCluster[i].core == 1) //core pt.
						       {
							  core_cluster.push_back(point);
							  core++; 
						       }   
						       
						       gCluster[rept].push_back(point); //Insert point Id to that cluster
					    }//end-IF   
				    
				}//end-for
				
				double t2_gClust=(double)clock()/CLOCKS_PER_SEC;
				
				//No. of clusters
					  int clusterNos = 0;
					      for(int i=0;i<gCluster.size();i++)
						  {
						       if(gCluster[i].size() > 0)
						         clusterNos++;
						  }	
				cout<<"#Clusters:"<<clusterNos<<"\n";
				
				cout<<"#Core clustered points:"<<core_cluster.size()<<";\n";
					  for(int i=0;i<core_cluster.size();i++) //Display core-clustered points
						{
							cout<<core_cluster[i]<<" ";	     
						}
					
					      cout<<endl;	
					  
				
				cout<<"#Non-core clustered points:"<<non_core_cluster.size()<<";\n";
				          for(int i=0;i<non_core_cluster.size();i++) //Display non-core-clustered points
						{
							cout<<non_core_cluster[i]<<" ";	     
						}
					
					      cout<<endl;	
					  
				
				
				cout<<"#Non-core noise points:"<<non_core_noise.size()<<"\n";
				          for(int i=0;i<non_core_noise.size();i++) //Display non-core-clustered points
						{
							cout<<non_core_noise[i]<<" ";	     
						}
					
					      cout<<endl;	
					  
				
				cout<<"\nThe graph clusters' vector is:"<<(t2_gClust-t1_gClust)<<" s"<<endl;			     
				          for(int i=0;i<nrow;i++) //sort the points in each cluster
						{
							cout<<"[Rep. point:"<<i<<"/Size:"<<gCluster[i].size()<<"]:";   
							  for(int j=0;j<gCluster[i].size();j++)
							     {
								 cout<<gCluster[i][j]<<" ";
							     }
							     
							cout<<endl<<endl;
						}
						
						
									
								
				
				                             // ***** Identification of LAYER for each point *****//
				                             
				                             
				double t1_layer=(double)clock()/CLOCKS_PER_SEC; 
				                             
				pLayer.resize(nrow); //find layers for each pt.                          
				unReco.resize(nrow); //not recommended connections
				
				Layer layer; //create 'layer' object
							                             
				for(int i=0;i<nrow;i++)
				{
				      int head = pCluster[i].rep; //Find the cluster head/rep.
				      int point_x = i; //source pt.(user)
				    
					    if(head == -1) //unclustered pt.
					     {					     
					        continue; // Noise Pts. will have no recommendations
					     }
					      
					    if(head >=0) //clustered pt.
					    {
					    
						      for(int j=0;j<gCluster[head].size();j++) //Target the cluster of 'point_x'
						      {
						          //cout<<gCluster[head][j]<<" ";						          
						          int point_y = gCluster[head][j]; //Fetch the cluster pt.					        
						            if(point_y == point_x) //same pt.
						              continue;
						              
						          double sim_link = SNG_matrix[point_x][point_y]; 
							    if(sim_link >= delta_link) //Layer-1 pts.; Similarity >= delta_link (0.5)
							    {
							         layer.layer_Num = 1; //Layer-1
							         layer.point_Num = point_y;
							         layer.edge_Sim = sim_link;
							         
							         pLayer[point_x].push_back(layer); //Insert Layer-1 & similarity 
							    }
							    
							    if(sim_link > 0 && sim_link < delta_link) //Layer-k pts.; 
							    {
							         double k_level;
							         k_level = ceil(delta_link/sim_link);
							         
							         layer.layer_Num = k_level; //Layer-k
							         layer.point_Num = point_y;
							         layer.edge_Sim = sim_link;
							         
							         pLayer[point_x].push_back(layer); //Insert Layer-k & similarity	
							         					             
							    }
							    
							    if(sim_link == 0)
							    {
							         layer.layer_Num = -1; //No layer assigned; not recommend
							         layer.point_Num = point_y;
							         layer.edge_Sim = sim_link;
							         
							         unReco[point_x].push_back(layer);							         
							    }
						            
						            
						      }//end for
						      
						      
						      //SORT layers
						      sort(pLayer[point_x].begin(), pLayer[point_x].end(), myComp);
						      
							 
					    }//end-IF
				    
				}//end for
				
				double t2_layer=(double)clock()/CLOCKS_PER_SEC;
				
					//Display K-Layers for pts.
					cout<<"Layer no./Point id./Similarity: "<<(t2_layer-t1_layer)<<" s"<<endl;
						for(int i=0; i<nrow; i++)
						{
						    cout<<"User #"<<i<<":\n";
							    for(int j=0; j<pLayer[i].size();j++)
							    {
								cout<<"[";
								cout<<pLayer[i][j].layer_Num<<"/";
								cout<<pLayer[i][j].point_Num<<"/";
								cout<<pLayer[i][j].edge_Sim<<"]";
							    }
							    
							    cout<<"\nUnrecommended pts.:";
							    for(int k=0;k<unReco[i].size();k++)
							    {
							        cout<<"[";
							        cout<<unReco[i][k].point_Num<<"/";
								cout<<unReco[i][k].edge_Sim<<"]";
							    }
							    
							    
							cout<<"\n---------\n";    
						}
					
					
				cout<<"Graph cluster + Layer formation time: "<<(t2_gClust-t1_gClust)+(t2_layer-t1_layer)<<" s"<<endl;	
						
	     return 0;
	     
  

   }//END___main()___
   
   
   
   //***FUNCTION
      // Find layers of each point
 










