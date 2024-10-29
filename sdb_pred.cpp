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

void write_values(string);
void write_binary_values(string);
//int link_strength(int,int);
void link_cluster(int,int);

//Declaring the parameters
int nrow,ncol;
int KNN;
double SNN;
int DENSITY;
double Time;

//Declaring the Data-Structures
vector< vector<double> >data_matrix;
vector< vector<double> >PC_matrix; 
vector< vector<double> >PoE_matrix;
vector< vector< pair<double,double> > >cosine_matrix;
vector< vector<double> >DC_matrix; 

vector< vector<double> >SNG_matrix; //total similarity matrix SNN graph or Social Net Graph
vector< vector< pair<int,double> > >KSNG_matrix; // residual KSNN graph; point index/similarity

//Core, non-core, noise pts.
vector<int>core;
vector<int>non_core;
vector<int>non_core_noise;

//Cluster data-structure
typedef struct Clust{

     public:
      int status;
      int rep;
      int core;
}Cluster;
Cluster *cluster;

//Binary search comparator for pair<x,y>
struct compare { 
	 bool operator()(const pair<double, int>& value, 
		const int& key) 
	 { 
	  return (value.second < key); 
	 } 
	 bool operator()(const int& key, 
	     const pair<double, int>& value) 
	 { 
	  return (key < value.second); 
	 } 
	 
};   


// Structure of the condition
// for sorting the pair by its
// second elements
struct myComp {
    constexpr bool operator()(
        pair<int, int> const& a,
        pair<int, int> const& b)
        const noexcept
    {
        return a.second < b.second;
    }
};

 

// START___ main()___
int main(int argc, char* argv[])
{
                                                           // ****** Load the dataset ******** //

	  double t1_data=(double)clock()/CLOCKS_PER_SEC;
	  string input_file = argv[1];
	  string input_file_bin = input_file+".bin";
	  string input_file_string = argv[1];
  
 
		  ifstream in(input_file_bin.c_str(),ios::in|ios::binary);
		  if(!in)
		  {cout<<input_file_bin<<" dataset file is not found check again!!"<<endl; exit(0);}
		  cout<<"Processed Input file:"<<input_file_bin<<endl;

			  //Read the nrow,ncol values from input binary file
			    in.read((char*)&nrow,sizeof(int));
			    in.read((char*)&ncol,sizeof(int));

			    cout<<"Rows:"<<nrow<<";Columns:"<<ncol<<endl;
			    data_matrix.resize(nrow);    

			  //Read the rest of the values from the input binary file
			  long double temp;
			  
			  for(int i=0;i<=nrow-1;i++)
			    {
			       for(int j=0;j<=ncol-1;j++)
			       {
				 in.read((char*)&temp,sizeof(long double));
				 data_matrix[i].push_back(temp);
			       }
			    }
			  double t2_data=(double)clock()/CLOCKS_PER_SEC;

				   cout<<"The data matrix is:"<<(t2_data-t1_data)<<" s"<<endl;
				   /*for(int i=0;i<=nrow-1;i++)
				   {
				      for(int j=0;j<=ncol-1;j++)
				      {
					 cout<<data_matrix[i][j]<<" ";
				      }
				    
				     cout<<endl;
				   }*/
				 
				   in.close();

   
                                       // *** Fixing the Parameters of the DB-SNN/SDB_pred Algorithm *** //
                             
  KNN = stoi(argv[2]);
  SNN = stod(argv[3]);
  DENSITY = stoi(argv[4]);

  cout<<"Algorithm parameters set:(KNN size="<<KNN<<" ; SNN threshold="<<SNN<<" ; DENSITY threshold="<<DENSITY<<")\n";
  cout<<"Running for #points: "<<nrow<<endl;


                                   //********** Load the P x C matrix; P = #persons; C = #Categories ************//


		 double t1_PC=(double)clock()/CLOCKS_PER_SEC; 
		 PC_matrix.resize(nrow);
		 
		 //for(int i=0;i<=nrow-1;i++)
		 //PC_matrix[i].resize(KNN);

		  for(int i=0;i<nrow;i++) //Assign data matrix to PC matrix
		   {
		       for(int j=0;j<ncol;j++)
		       {
			   PC_matrix[i].push_back(data_matrix[i][j]);
		       }
		   }
		   
		   double t2_PC=(double)clock()/CLOCKS_PER_SEC; 
		   
		   //Display PC_matrix
		      cout<<"The P x C matrix is: "<<(t2_PC-t1_PC)<<" s"<<endl;
		      /*for(int i=0;i<nrow;i++)
			   {
			      for(int j=0;j<ncol;j++)
			      {
				 cout<<PC_matrix[i][j]<<" ";
			      }
			    
			     cout<<endl;
			   }*/
   

 
                                        //************PoE matrix creation (Probability of Engagement) *************//
                                        
         double t1_poe=(double)clock()/CLOCKS_PER_SEC; 
         PoE_matrix.resize(nrow);  
                 
         double w,poe;
         for(int i=0;i<nrow;i++) 
	   {
	       for(int j=0;j<ncol;j++)
	       {
		   w = data_matrix[i][j];
		   poe = w/10.0;
		   
		   PoE_matrix[i].push_back(poe);
	       }
	   }
                                    
         double t2_poe=(double)clock()/CLOCKS_PER_SEC;
         
         //Display PoE_matrix
         cout<<"The PoE matrix is: "<<(t2_poe-t1_poe)<<" s"<<endl;
         
          /*for(int i=0;i<nrow;i++)
	   {
	      for(int j=0;j<ncol;j++)
	      {
		 cout<<PoE_matrix[i][j]<<" ";
	      }
	    
	     cout<<endl;
	   }*/                                
                                         
                                         
                                         
                                         
                                         
                                               //************ Cosine similarity matrix construction *************//
                                               
                                               
           double t1_cos=(double)clock()/CLOCKS_PER_SEC;  
                                        
           //Cosine similarity matrix calculation
           //vector< vector< pair<double,double> > >cosine_matrix; 
           cosine_matrix.resize(nrow);      
           double S_c, Angle;
           
           for(int i=0;i<nrow;i++)
           {
               for(int j=0;j<nrow;j++)
               {   
		             double dotp = 0.0; //Find dot product (numerator) of cosine sim.              
				 for(int k=0;k<KNN;k++)
				  {
				      dotp += (PoE_matrix[i][k]*PoE_matrix[j][k]); //sum of products-cosine numerator    
				  }  
				  
			     double modPx, modPy;
			     double sum_sqr_Px = 0.0, sum_sqr_Py = 0.0;
			     for(int k=0;k<KNN;k++) //Find product of mod. (dinominator) of cosine sim.
			     {
			         sum_sqr_Px += pow(PoE_matrix[i][k],2);
			         sum_sqr_Py += pow(PoE_matrix[j][k],2);
			     }
			     
			     modPx = sqrt(sum_sqr_Px);
			     modPy = sqrt(sum_sqr_Py);
		             
                             S_c = dotp/(modPx*modPy); //Cosine similarity
                             Angle = acos(S_c); //Angle in radians
                             
                             cosine_matrix[i].push_back(pair<double,double>(S_c, Angle)); //Assign the cosine sim., angle to the cosine matrix
                                    
                                      
               }//end for
               
           
           }//end for
           
           double t2_cos=(double)clock()/CLOCKS_PER_SEC;
           
           //Display cosine_matrix (S_c)
           cout<<"The cosine similarity matrix is: "<<(t2_cos-t1_cos)<<" s"<<endl;         
              /*for(int i=0;i<nrow;i++)
	      {
	           cout<<"User:"<<i<<"(cosine sim/radians):\n";
		      for(int j=0;j<nrow;j++)
		      {
			   cout<<"("<<i<<","<<j<<"):";
			   cout<<"["<<cosine_matrix[i][j].first<<"/"<<cosine_matrix[i][j].second<<"];\n";
		      }
		    
		     cout<<"------------------------------------ \n";
	      } */                            
                         
           
           
                                         
                                         //************ Dominant category matrix construction *************//
                                         
                                         
           double t1_DC=(double)clock()/CLOCKS_PER_SEC;  
                                        
           //Dominant category matrix calculation
           //vector< vector<double> >DC_matrix; 
           DC_matrix.resize(nrow); 
           
           vector< pair<double,int> >Dx;
           vector< pair<double,int> >Dy;
           
           double max=0, poe_x, poe_y;
           float delta_dom = 0.65;
           double S_d;
           
           for(int i=0;i<nrow;i++) //Find the Dominant category of pts. Px, Py
           {
               //cout<<i<<endl;
               for(int j=0;j<nrow;j++)
               {
               
		           for(int k=0;k<KNN;k++) 
		           {
		               poe_x = PoE_matrix[i][k];
		               poe_y = PoE_matrix[j][k]; 
		               if(poe_x > delta_dom) //poe value of i^th category for Pt. x is greater than spec. threshold
		               {
		                   Dx.push_back(pair<double,int>(poe_x,k)); //Dominant set Dx of pt. Px                          
		               }       
		               
		               if(poe_y > delta_dom) //poe value of i^th category for Pt. y is greater than spec. threshold
		               {
		                   Dy.push_back(pair<double,int>(poe_y,k)); //Dominant set Dy of pt. Py                          
		               }                         
		               
		           }//end for
                   
                                     
						//Display Dx, Dy for each pair
						   /*cout<<endl<<i<<"] ";
						   for(int dx=0; dx<Dx.size();dx++)
						   {
							cout<<Dx[dx].first<<"/C"<<Dx[dx].second<<" ";
						   } 
						   
						   cout<<endl<<j<<"] ";
						   for(int dy=0; dy<Dy.size();dy++)
						   {
							cout<<Dy[dy].first<<"/C"<<Dy[dy].second<<" ";
						   }*/
						   
					   
					   
			//Find intersection between Dx and Dy
			vector<int>common_DxDy;
			
			for(int dx=0; dx<Dx.size(); dx++)
			{
			    int elem = Dx[dx].second;
			    if(binary_search(Dy.begin(), Dy.end(), elem, compare())) //Bin search in Dy for elem.
			    {
			       common_DxDy.push_back(elem); //common dominant elements are identified 
			    }  		   
			}			   
			
			
					          //Display intersection elements betwn. Dx, Dy
					          /*cout<<"\n("<<i<<","<<j<<"): {";
					       	  for(int t=0;t<common_DxDy.size();t++)
					       	  { 		  
					       	      cout<<common_DxDy[t]<<" ";       	
						  }		   
						  cout<<"}";
						  cout<<"\n-------------";*/
						  
						  						  
			   
			   //Calculating S_d value 	
			   if(Dx.size() == 0 && Dy.size() == 0) //Both the dominant sets are empty
			   {
			           S_d = 0.0;
			      	   DC_matrix[i].push_back(S_d);
			   }
			   else
			   {			   		   
				   S_d = (double)(2*common_DxDy.size())/(Dx.size()+Dy.size());
				   DC_matrix[i].push_back(S_d);
			   }
					  	   
						   
		           common_DxDy.erase(common_DxDy.begin(),common_DxDy.end());
		           Dx.erase(Dx.begin(),Dx.end());
		           Dy.erase(Dy.begin(),Dy.end());
                   
                }//end for 'j' loop
               
               
           }//end for 'i' loop
           
           double t2_DC=(double)clock()/CLOCKS_PER_SEC;
           
            //Display DC_matrix (S_d)
             cout<<"The Dominant category matrix is: "<<(t2_DC-t1_DC)<<" s"<<endl;         
             /*for(int i=0;i<nrow;i++)	      
             {
	           
		      for(int j=0;j<nrow;j++)
		      {
			   cout<<"("<<i<<","<<j<<"):";
			   cout<<"["<<DC_matrix[i][j]<<"];\n";
		      }
		    
		     cout<<"------------------------------------\n";
	      }*/            
	      
	      
	      
	      
	           
                                                     //****************** Total/K-sparsified Similarity matrix creation **************//
                                                     
                                                     
              double t1_KSNG=(double)clock()/CLOCKS_PER_SEC;                                       
              SNG_matrix.resize(nrow);
              KSNG_matrix.resize(nrow);
              
              double delta_link;
              for(int i=0; i<nrow; i++)
              {
                //cout<<"\nUser: "<<i<<endl; 
                   for(int j=0; j<nrow; j++)
                   {
                        if(i==j){
                          SNG_matrix[i].push_back(1.0); //Self similarity is fixed to be 1.0
                          continue;
                        }
                        
                        S_c = cosine_matrix[i][j].first;
                        S_d = DC_matrix[i][j];
                        
                        //cout<<"("<<i<<","<<j<<"):";
                        //cout<<S_c<<"/"<<S_d<<endl;
                        
                        delta_link = (double)(S_c*S_d);
                        SNG_matrix[i].push_back(delta_link);
                        
                        
                        if(SNG_matrix[i][j] >= SNN)
                        {
                           
                           KSNG_matrix[i].push_back(pair<int,double>(j,delta_link));
                           //cout<<"["<<j<<"/"<<delta_link<<"] ";
                        }
                        
                   }
              }
              
              double t2_KSNG=(double)clock()/CLOCKS_PER_SEC;  
              
              cout<<"The SNG/K-SNG similarity matrix is: "<<(t2_KSNG-t1_KSNG)<<" s"<<endl;  
             //Display the SNG_matrix (S_c*S_d)                   
             /*for(int i=0;i<nrow;i++)	      
             {
	          cout<<"User #"<<i<<endl; 
		      for(int j=0;j<nrow;j++)
		      {
			   cout<<"("<<i<<","<<j<<"):";
			   cout<<"["<<SNG_matrix[i][j]<<"];";
		      }
		    
		     cout<<"\n------------------------------------\n";
	      } */          
              
              
              
              
                                                              // K-sparsified similarity matrix display */
                                                              
              //Display the K-SNG matrix                                                
              /*for(int i=0;i<nrow;i++)	      
              {
	          cout<<"User #"<<i<<" ("<<KSNG_matrix[i].size()<<" adjacent links)"<<endl; 
		      for(int j=0;j<KSNG_matrix[i].size();j++)
		      {
			  cout<<"["<<KSNG_matrix[i][j].first<<"/"<<KSNG_matrix[i][j].second<<"]; ";
		      }
		    
		     cout<<"\n------------------------------------\n";
	      }*/                
                        
                        
                        
                        
                         
                                                     //*********Core (Dense)/Non-Core (Non-dense) pt. detection**********//
                                                     
                                                     
                   //Core Non-Core detection
		       double t1_core=(double)clock()/CLOCKS_PER_SEC;
		       for(int i=0;i<nrow;i++)
		       {
			  if(KSNG_matrix[i].size() > DENSITY)
			    core.push_back(i);
			  else
			    non_core.push_back(i); 
		       }
		       double t2_core=(double)clock()/CLOCKS_PER_SEC;
		       cout<<"The Core & Non-core points:"<<(t2_core-t1_core)<<" s"<<endl;
		       
		           //Display core points
		                cout<<"#Core points:("<<core.size()<<")\n";
				    /*for(int i=0;i<core.size();i++)
				       {
					 cout<<core[i]<<" ";

				       }*/
				       
				       //Display Non-core points
		                         cout<<"#Non-Core points:("<<non_core.size()<<")\n";			
					   /*for(int i=0;i<non_core.size();i++)
					     {
					        cout<<non_core[i]<<" ";
					     }*/
					     
					     
					   
					   
					   
					                  
					                   //************CLUSTER FORMATION**************//
					                   
			        //Initial clusters
			         cluster = new Cluster[nrow];    
				       for(int i=0;i<nrow;i++) //Initial clusters
					  {
						  if(KSNG_matrix[i].size() > DENSITY)// core pt. check
						  {
						     cluster[i].status=1; //  '1' clustered - '0' not clustered
						     cluster[i].rep=i;    //  'i' is the representative point
						     cluster[i].core=1;   //  '1' is core - '0' is non-core 
						  }
						  else
						  {
						     cluster[i].status=0;
						     cluster[i].rep=-1;   //  '-1' means no representative point
						     cluster[i].core=0;

						  }
						
					  }// end for	
				  
				  
					       //Display initial clusters 
						   cout<<"Initial Clusters:\n";
						     /*for(int i=0;i<nrow;i++)
						       {
							     cout<<i<<"] ";
							     cout<<cluster[i].status<<"/";
							     cout<<cluster[i].rep<<"/";
							     cout<<cluster[i].core<<endl;

						       }*/			                   
					                   
					                   
					                   
		                    //******Case When No Clusters are formed******//
                                    if(core.size()==0)
				      {
				          double t1_cluster, t2_cluster;
				          t1_cluster = t2_cluster = 0;
				          
					  cout<<"\nNo additional clusters possible: "<<(t2_cluster-t1_cluster)<<" s";
					  
					       double t1_noise=(double)clock()/CLOCKS_PER_SEC;      
						       //int noise=0;
						       for(int i=0;i<nrow;i++)
						       {
							  if(cluster[i].core==0 && cluster[i].rep==-1)
							   {
							     //cout<<i<<" ";
							        non_core_noise.push_back(i);    
							   }
						       }
						
						double t2_noise=(double)clock()/CLOCKS_PER_SEC;        
						cout<<endl<<"Noise points"<<"("<<non_core_noise.size()<<"): ";
					        cout<<(t2_noise-t1_noise)<<" s\n";
					        
					        
					                                             // **** Algorithm <SDB_pred> LOGS **** 
					        
					        //Total TIME required for the <SDB_pred> algorithm:
					        Time = (t2_PC-t1_PC)+(t2_poe-t1_poe)+(t2_cos-t1_cos)+(t2_DC-t1_DC)+(t2_KSNG-t1_KSNG)+(t2_core-t1_core)+
                                                       (t2_cluster-t1_cluster)+(t2_noise-t1_noise);
                                                       
                                                
                                                //<Total Time taken for SDB_pred algorithm>
                                                cout<<endl<<"Total time elapsed: "<<Time<<" s"<<endl;
                                                
                                                    //Write LOG values
					            ofstream  time_SDB_pred("Time_SDB_pred",ios::out);
					            time_SDB_pred<<"#Rows: "<<nrow<<"#Col: "<<ncol<<endl;
					            time_SDB_pred<<"P x C matrix load time: "<<(t2_PC-t1_PC)<<" s"<<endl;
					            time_SDB_pred<<"PoE matrix build time: "<<(t2_poe-t1_poe)<<" s"<<endl;
					            time_SDB_pred<<"Cosine matrix build time: "<<(t2_cos-t1_cos)<<" s"<<endl;
					            time_SDB_pred<<"Dominant category matrix build time: "<<(t2_DC-t1_DC)<<" s"<<endl;
					            time_SDB_pred<<"SNG/KSNG matrix build time: "<<(t2_KSNG-t1_KSNG)<<" s"<<endl;
					            time_SDB_pred<<"Core/non-core points build time: "<<(t2_core-t1_core)<<" s"<<endl;
					            time_SDB_pred<<"Cluster(s) build time: "<<(t2_cluster-t1_cluster)<<" s"<<endl;
					            time_SDB_pred<<"Noise build time: "<<(t2_noise-t1_noise)<<" s"<<endl;
					            
					            time_SDB_pred<<"Total time (SDB_pred): "<<Time<<" s"<<endl;
					            time_SDB_pred<<"-------\n";
					            time_SDB_pred.close();				       					        
					       				     
						            							            
							       //Memory Footprint
								     ofstream mem_ni("Mem_SDB_pred",ios::app);
								     void process_mem_usage(double&, double&);
								     double vm, rss;
								     process_mem_usage(vm, rss);
								     mem_ni << vm <<" "<< rss << endl;
								     
								     
								     
						  //Writing values...
						       cout<<endl<<"Writing values...: ";
						       double t1_write=(double)clock()/CLOCKS_PER_SEC;						       
							    write_values(input_file_string);//func. call							  
						       double t2_write=(double)clock()/CLOCKS_PER_SEC;							       
						       cout<<(t2_write-t1_write)<<" s"<<endl;	               
					                                                                    
                                                       					               
							       // Write string values
							            //write_values(input_file_string);
					               
                                                     
					       
					        //Program exits here (when NO Clusters formed)
					        exit(0);   
					        
					    
				       }// end of IF-block 
				       
				       				                					                   
			                
			       //Core pt. cluster formation   	
			       double t1_cluster=(double)clock()/CLOCKS_PER_SEC;
			       vector<int>temp_cluster;
                               int min; 
                               for(int i=0;i<nrow;i++)
                               {
                                   
                                      if(KSNG_matrix[i].size() > DENSITY)// core pt. check
		                      {
		                      
				                    for(int j=0;j<KSNG_matrix[i].size();j++)
						     {
							if(KSNG_matrix[KSNG_matrix[i][j].first].size() > DENSITY)// core pt. check
							{
							   temp_cluster.push_back(cluster[KSNG_matrix[i][j].first].rep);
							}                  
						     }
		                      
		                      
		                      
						      if(temp_cluster.size() == 0)//(single core pt. cluster); no link with other core pts.
						       {
								 cluster[i].status = 1;
								 cluster[i].rep = i;
								 cluster[i].core = 1;
								 continue;
						       }
		                      
						      min = *min_element(temp_cluster.begin(),temp_cluster.end()); // find the min. representative of all pts.
						      
						      if(cluster[i].rep < min)
						      { 
								cluster[i].rep = cluster[i].rep;
								cluster[i].status = 1;
								for(int j=0;j<KSNG_matrix[i].size();j++)
								{
								   if(KSNG_matrix[KSNG_matrix[i][j].first].size() > DENSITY)// core pt. check
									{
									   link_cluster(cluster[KSNG_matrix[i][j].first].rep,cluster[i].rep);
									   cluster[KSNG_matrix[i][j].first].rep = cluster[i].rep;
									   cluster[KSNG_matrix[i][j].first].status = 1; 
									}

								}

						       }//end-IF
						       
						       if(cluster[i].rep >= min)
							 { 
								link_cluster(cluster[i].rep,min);
								cluster[i].rep = min;
								for(int j=0;j<KSNG_matrix[i].size();j++)
								{
								   if(KSNG_matrix[KSNG_matrix[i][j].first].size() > DENSITY)// core pt. check
									{
								           link_cluster(cluster[KSNG_matrix[i][j].first].rep,min);
									   cluster[KSNG_matrix[i][j].first].rep = min;
								           cluster[KSNG_matrix[i][j].first].status = 1;
									}

								}

							  }//end-IF							       
							       						      	       
						      							       
				              
		                      }//end-IF (end of core pt. cluster formation)
		                     
		                     
		                     temp_cluster.erase(temp_cluster.begin(),temp_cluster.end());
		                     
                               
                           }//end for
                           
                           
                           
                           //Non-Core pt. cluster formation
                           typedef pair<int,double> pd;
                           priority_queue<pd, vector<pd>, myComp>nearest_core;
                                                      
                           int nearest;
			   for(int i=0;i<nrow;i++)
			   {
			            
			              if(KSNG_matrix[i].size() <= DENSITY)// non-core pt. check
		                      {
		                           if(KSNG_matrix[i].size() == 0)
                                             continue;
                                             
				           
				           for(int j=0;j<KSNG_matrix[i].size();j++)
					     {
						if(KSNG_matrix[KSNG_matrix[i][j].first].size() > DENSITY)// core pt. check of adj. node
						nearest_core.push(pair<int,double>(KSNG_matrix[i][j].first,KSNG_matrix[i][j].second));		        
					     }
					     
					     if(nearest_core.size() > 0) //no. of nearest core pts.
					     {
						     nearest = nearest_core.top().first;
						     cluster[i].rep = cluster[nearest].rep;
						     cluster[i].status = 1;
					     } 
					     
		                            
		                      }//end-IF (end of non-core pt. cluster formation)
		                      
		                      
		                      
		                      while(!nearest_core.empty()) //Empty the priority queue  
                                        nearest_core.pop();
		                      
			   
			   }// end for
			   
			   double t2_cluster=(double)clock()/CLOCKS_PER_SEC; 			   
			 				  
				   
				   //Display clusters
				   cout<<"Final Clusters: "<<(t2_cluster-t1_cluster)<<" s"<<endl;
				      /*for(int i=0;i<nrow;i++)
				       {
					     cout<<i<<"] ";
					     cout<<cluster[i].status<<"/";
					     cout<<cluster[i].rep<<"/";
					     cout<<cluster[i].core<<endl;

				       }*/
				       
				       
				       
				           
				                        		      //***** Noise points *****//
				                        			
				                        
			       double t1_noise=(double)clock()/CLOCKS_PER_SEC;    
			         
			       for(int i=0;i<nrow;i++)
			       {
				  if(cluster[i].core==0 && cluster[i].rep==-1)
				   {
				       //cout<<i<<" ";
				        non_core_noise.push_back(i);    
				   }
			       }			       
			       			       
			       double t2_noise=(double)clock()/CLOCKS_PER_SEC;        
				cout<<"Noise points"<<"("<<non_core_noise.size()<<"): ";
			        cout<<(t2_noise-t1_noise)<<" s"<<endl;	  
			        
			        
			        
			                              
                                                                        // **** Algorithm <SDB_pred> Final LOGS ****//
                                                                              
                                          
                              //Total TIME required for the <SDB_pred> algorithm:
				Time = (t2_PC-t1_PC)+(t2_poe-t1_poe)+(t2_cos-t1_cos)+(t2_DC-t1_DC)+(t2_KSNG-t1_KSNG)+(t2_core-t1_core)+
		                       (t2_cluster-t1_cluster)+(t2_noise-t1_noise);      
		                               
		                               
		                   //<Total Time taken for SDB_pred algorithm>
                                           cout<<endl<<"Total time elapsed: "<<Time<<" s"<<endl;   
                                           
                                           
                                             //Write LOG values
						    ofstream  time_SDB_pred("Time_SDB_pred",ios::out);
						    time_SDB_pred<<"#Rows: "<<nrow<<"#Cols: "<<ncol<<endl;
						    time_SDB_pred<<"P x C matrix load time: "<<(t2_PC-t1_PC)<<" s"<<endl;
						    time_SDB_pred<<"PoE matrix build time: "<<(t2_poe-t1_poe)<<" s"<<endl;
						    time_SDB_pred<<"Cosine matrix build time: "<<(t2_cos-t1_cos)<<" s"<<endl;
						    time_SDB_pred<<"Dominant category matrix build time: "<<(t2_DC-t1_DC)<<" s"<<endl;
						    time_SDB_pred<<"SNG/KSNG matrix build time: "<<(t2_KSNG-t1_KSNG)<<" s"<<endl;
						    time_SDB_pred<<"Core/non-core points build time: "<<(t2_core-t1_core)<<" s"<<endl;
						    time_SDB_pred<<"Cluster(s) build time: "<<(t2_cluster-t1_cluster)<<" s"<<endl;
						    time_SDB_pred<<"Noise build time: "<<(t2_noise-t1_noise)<<" s"<<endl;                                       
					       			       					       
						    time_SDB_pred<<"Total time (SDB_pred): "<<Time<<" s"<<endl;
						    time_SDB_pred<<"-------\n";
						    time_SDB_pred.close();
						    	
					       
					         		//Memory Footprint
								     ofstream mem_ni("Mem_SDB_pred",ios::app);
								     void process_mem_usage(double&, double&);
								     double vm, rss;
								     process_mem_usage(vm, rss);
								     mem_ni << vm <<" "<< rss << endl;
								     
								     
						  //Writing values...
						       cout<<endl<<"Writing values...: ";
						       double t1_write=(double)clock()/CLOCKS_PER_SEC;						       
							    write_values(input_file_string);//func. call							  
						       double t2_write=(double)clock()/CLOCKS_PER_SEC;							       
						       cout<<(t2_write-t1_write)<<" s"<<endl;
					    
                               		       	                   
		       
                                                              
                return 0;
     
     
   }//END___main()___ 

//*** FUNCTION
  // LINK Clusters
	void link_cluster(int rp,int rq)
	{
	   if(rp<rq)
	   {
	      for(int i=0;i<=nrow-1;i++)  
	      {
		if(cluster[i].rep==rq)
		 cluster[i].rep=rp;
	      }
	   }

	   if(rq<rp)
	   {
	      for(int i=0;i<=nrow-1;i++)  
	      {
		if(cluster[i].rep==rp)
		 cluster[i].rep=rq;
	      }
	   }
	}//end func.
	
	

//*** FUNCTION
  // WRITE values to ascii/binary file
       void write_values(string input_file_string)
       {
       
			     //Dump the KNN value
				 ofstream out_knn_global("KNN");
				   if(!out_knn_global)
				    {
				      cout<<"Cannot open the KNN file\n"<<endl;
				      exit(0);
				    }
				    
				    out_knn_global<<KNN;
				    out_knn_global.close();  
				    
				    			//Dump the KNN binary value
				    			      ofstream knn_binary("KNN.bin",ios::out|ios::binary);								   
								   if(!knn_binary)
								    {
								      cout<<"Cannot open the KNN.bin file"<<endl;
								      exit(0);
								    }

								   knn_binary.write((char*)&KNN,sizeof(KNN)); 
								   knn_binary.close(); 
				    
				    
				    
			    //Dump the SNN value
			       ofstream out_snn_global("SNN");
				   if(!out_snn_global)
				    {
				      cout<<"Cannot open the SNN file\n"<<endl;
				      exit(0);
				    }	    
				    
			       out_snn_global<<SNN;
                               out_snn_global.close();                              
                               	
                               			     //Dump the SNN binary value
							   ofstream snn_binary("SNN.bin",ios::out|ios::binary);
								   if(!snn_binary)
								    {
								      cout<<"Cannot open the SNN.bin file\n"<<endl;
								      exit(0);
								    }	    
							    
							    snn_binary.write((char*)&SNN,sizeof(SNN)); 
							    snn_binary.close();
                               
                               
                               
                            //Dump the DENSITY value
                                ofstream out_density_global("DENSITY");
				   if(!out_density_global)
				    {
				      cout<<"Cannot open the DENSITY file\n"<<endl;
				      exit(0);
				    }
				    
				    //cout<<"\nDensity:"<<DENSITY;
				    out_density_global<<DENSITY;
   				    out_density_global.close();
   				                               				    
   				       				//Dump the DENSITY binary value
									ofstream density_binary("DENSITY.bin",ios::out|ios::binary);
									   if(!density_binary)
									    {
									      cout<<"Cannot open the DENSITY.bin file\n"<<endl;
									      exit(0);
									    }
									    
									   density_binary.write((char*)&DENSITY,sizeof(DENSITY)); 
									   density_binary.close();
									   	    
   				       				    
   			      // Dump data_matrix, PC_matrix not required	 			                      		         
				    				         
				         
			      //Dump the PoE_matrix
			      ofstream out_PoE_matrix("PoE_matrix");
				 if(!out_PoE_matrix)
				    {
				      cout<<"Cannot open the PoE_matrix file\n"<<endl;
				      exit(0);
				    }
				    
				    	 for(int i=0;i<nrow;i++)
					 {
						      
					      for(int j=0;j<ncol;j++)
					      {
					         out_PoE_matrix<<PoE_matrix[i][j]<<" ";			       

					      }						      
					  
					       out_PoE_matrix<<endl;

					  }

				         out_PoE_matrix.close();	
				         
				                                //Dump PoE_matrix binary
				                                ofstream PoE_matrix_binary;
								   PoE_matrix_binary.open("PoE_matrix.bin",ios::out|ios::binary);
								   if(!PoE_matrix_binary)
								    {
								      cout<<"Cannot open the PoE_matrix.bin file\n"<<endl;
								      exit(0);
								    }
								    
								       double temp_PoE;									   
									   for(int i=0;i<nrow;i++)
									    {
									       for(int j=0;j<ncol;j++)
									       {
									          temp_PoE = PoE_matrix[i][j]; //Read from PoE matrix
										  PoE_matrix_binary.write((char*)&temp_PoE,sizeof(temp_PoE));//
									       }
									    }

									   PoE_matrix_binary.close();      	         
				    
				    
				    
			      		 
					  
			      //Dump cosine_matrix values
			         ofstream out_cosine_matrix("cosine_matrix");
				    if(!out_cosine_matrix)
				    {
				      cout<<"Cannot open the Cosine_matrix file\n"<<endl;
				      exit(0);
				    }			    
				    
				      out_cosine_matrix<<"Pair(x,y):[S_c value (cosine similarity)/angle (radians)]\n";				        
					 for(int i=0;i<nrow;i++)
					 {
					    out_cosine_matrix<<"User #"<<i<<":\n";					    	      
					      for(int j=0;j<nrow;j++)
					      {
					       out_cosine_matrix<<"("<<i<<","<<j<<"):";
					       out_cosine_matrix<<"["<<cosine_matrix[i][j].first<<"/";
					       out_cosine_matrix<<cosine_matrix[i][j].second<<"] ";

					      }						      
					  
					       out_cosine_matrix<<endl<<"----------"<<endl;

					  }

				         out_cosine_matrix.close();	  
				         
				                                   //Dump Cosine_matrix binary
				                                   ofstream cosine_matrix_binary;
								   cosine_matrix_binary.open("cosine_matrix.bin",ios::out|ios::binary);
								   if(!cosine_matrix_binary)
								    {
								      cout<<"Cannot open the cosine_matrix.bin file\n"<<endl;
								      exit(0);
								    }
								    
								       pair<double,double>temp_cos;
									   //Read and Write other values
									   for(int i=0;i<nrow;i++)
									    {
									       for(int j=0;j<nrow;j++)
									       {
									          temp_cos.first = cosine_matrix[i][j].first;
									          temp_cos.second = cosine_matrix[i][j].second;
										  cosine_matrix_binary.write((char*)&temp_cos,sizeof(temp_cos));
									       }
									    }

									   cosine_matrix_binary.close();    
				         



                                //Dump DC_matrix values 
                                   ofstream out_DC_matrix("DC_matrix");
				     if(!out_DC_matrix)
				       {
					   cout<<"Cannot open the DC_matrix file\n"<<endl;
					   exit(0);
				       }		       
				       	
				       	out_DC_matrix<<"Pair(x,y):[S_d value (dominant categories similarity)] \n";			     
				       	 for(int i=0;i<nrow;i++)
					    {
					         out_DC_matrix<<"User #"<<i<<":\n";
						      for(int j=0;j<nrow;j++)
						      {
						          out_DC_matrix<<"("<<i<<","<<j<<"):";
						          out_DC_matrix<<"["<<DC_matrix[i][j]<<"] ";
						      }
						   
							 out_DC_matrix<<endl<<"----------"<<endl;

					    }

				          out_DC_matrix.close();	
				                          
				                                 //Dump DC_matrix binary
				                                   ofstream DC_matrix_binary;
								   DC_matrix_binary.open("DC_matrix.bin",ios::out|ios::binary);
								   if(!DC_matrix_binary)
								    {
								      cout<<"Cannot open the DC_matrix.bin file\n"<<endl;
								      exit(0);
								    }
								    
								       double temp_DC;
									   //Read and Write other values
									   for(int i=0;i<nrow;i++)
									    {
									       for(int j=0;j<nrow;j++)
									       {
									          temp_DC = DC_matrix[i][j];
										  DC_matrix_binary.write((char*)&temp_DC,sizeof(temp_DC));
									       }
									    }

									   DC_matrix_binary.close(); 			                           
				                           
				                           				         				         
                                                                
                                   //Dump the SNG or total_similarity_matrix
                                      ofstream out_SNG_matrix("SNG_matrix");
					    if(!out_SNG_matrix)
					    {
					      cout<<"Cannot open the SNG_matrix file\n"<<endl;
					      exit(0);
					    }
				    
				              out_SNG_matrix<<"Pair(x,y):[S_c*S_d value (final similarity)] \n";
					      for(int i=0;i<nrow;i++)
				       		{
						      out_SNG_matrix<<"User #"<<i<<":\n";
							  for(int j=0;j<SNG_matrix[i].size();j++)
							  {
							     out_SNG_matrix<<"("<<i<<","<<j<<"):";
							     out_SNG_matrix<<"["<<SNG_matrix[i][j]<<"] ";
							  }
						
							  out_SNG_matrix<<endl<<"----------"<<endl;
				       		} 
				       		
				       		out_SNG_matrix.close();
				       		
			       		                          //Dump SNG_matrix binary
					                           ofstream SNG_matrix_binary;
								   SNG_matrix_binary.open("SNG_matrix.bin",ios::out|ios::binary);
								   if(!SNG_matrix_binary)
								    {
								      cout<<"Cannot open the SNG_matrix.bin file\n"<<endl;
								      exit(0);
								    }
								    
								       double temp_SNG;								   
									   //Read and Write other values
									   for(int i=0;i<nrow;i++)
									    {
									          int row_size=SNG_matrix[i].size();
                                                                                  SNG_matrix_binary.write((char*)&row_size,sizeof(row_size));
										       for(int j=0;j<nrow;j++)
										       {
											  temp_SNG = SNG_matrix[i][j];
											  SNG_matrix_binary.write((char*)&temp_SNG,sizeof(temp_SNG));
										       }
									    }

									   SNG_matrix_binary.close(); 	
				       		
			       		
			       		
			       		
			       	//Dump the KSNG or K-sparsified_similarity_matrix
			       	      ofstream out_KSNG_matrix("KSNG_matrix");
					    if(!out_KSNG_matrix)
					    {
					      cout<<"Cannot open the KSNG_matrix file\n"<<endl;
					      exit(0);
					    }	
					    
					       out_KSNG_matrix<<"(User #x, Link #n):[User #y/strong link similarity] \n";
				       	       for(int i=0;i<nrow;i++)
				       		{
						     out_KSNG_matrix<<"User #"<<i<<" ("<<KSNG_matrix[i].size()<<" adjacent links)\n";
						       for(int j=0;j<KSNG_matrix[i].size();j++)
							  {
							     out_KSNG_matrix<<"("<<i<<","<<j<<"):";
							     out_KSNG_matrix<<"["<<KSNG_matrix[i][j].first<<"/";
							     out_KSNG_matrix<<KSNG_matrix[i][j].second<<"] ";
							  }
						
							  out_KSNG_matrix<<endl<<"----------"<<endl;
				       		} 
				       		
				       		out_KSNG_matrix.close();
				       		
			       		                         //Dump KSNG_matrix binary
					                           ofstream KSNG_matrix_binary;
								   KSNG_matrix_binary.open("KSNG_matrix.bin",ios::out|ios::binary);
								   if(!SNG_matrix_binary)
								    {
								      cout<<"Cannot open the KSNG_matrix.bin file\n"<<endl;
								      exit(0);
								    }
								    
								       pair<int,double>temp_KSNG;
									   //Read and Write other values
									   for(int i=0;i<nrow;i++)
									    {
									       int row_size = KSNG_matrix[i].size();
                                                                               KSNG_matrix_binary.write((char*)&row_size,sizeof(row_size));
										       for(int j=0;j<KSNG_matrix[i].size();j++)
										       {
											  temp_KSNG.first = KSNG_matrix[i][j].first;
											  temp_KSNG.second = KSNG_matrix[i][j].second;
											  KSNG_matrix_binary.write((char*)&temp_KSNG,sizeof(temp_KSNG));
										       }
									    }

									   KSNG_matrix_binary.close(); 	
				       		
				       		
			       		
			       		
			       		
			       		
			       	//Dump core points values
			       	     ofstream out_core("core_points");
					if(!out_core)
					    {
					      cout<<"Cannot open the core_points file\n"<<endl;
					      exit(0);
					    }	
					    
					
					    if(core.size() == 0)out_core<<"";
					    if(core.size() > 0)
					    {

						    for(int i=0;i<core.size();i++)
						     {
						         out_core<<core[i]<<endl;	      
						     }					  
							
					    }
					    
					    out_core.close();
					    
							               //Dump core points binary
									   ofstream core_binary("core_points.bin",ios::out|ios::binary);
									    if(!core_binary)
									    {
									      cout<<"Cannot open the core_points.bin file\n"<<endl;
									      exit(0);
									    }
									    int a;
									    if(core.size() == 0)core_binary.write((char*)&a,0);
									   
									    if(core.size() > 0){
										    for(int i=0;i<core.size();i++)
										     {
										       core_binary.write((char*)&core[i],sizeof(core[i]));	      
										     }
									      }
									    core_binary.close();
					    
					    
   			         //Dump non-core points values
   			              ofstream out_non_core("non_core_points");
					 if(!out_non_core)
					    {
					      cout<<"Cannot open the non_core_points file\n"<<endl;
					      exit(0);
					    }
					    
					    
				      if(non_core.size() == 0){out_non_core<<"";}
				      if(non_core.size() > 0)
				      {

					    for(int i=0;i<non_core.size();i++)
					     {
					       out_non_core<<non_core[i]<<endl;	      
					     }
					     					    
				      }				      
				      
				      out_non_core.close();
				      
					                     //Dump non-core points binary
								 ofstream non_core_binary("non_core_points.bin",ios::out|ios::binary);
								    if(!non_core_binary)
								    {
								      cout<<"Cannot open the non_core_points.bin file\n"<<endl;
								      exit(0);
								    }

								    if(non_core.size() == 0)non_core_binary.write((char*)&a,0);
								    if(non_core.size() > 0){
									    for(int i=0;i<non_core.size();i++)
									     {
									       non_core_binary.write((char*)&non_core[i],sizeof(non_core[i]));	      
									     }
								       }
								     
								   non_core_binary.close();
				      
				      
				      
				      
				 //Dump cluster values
				      ofstream out_cluster_global("Cluster_global");
					  if(!out_cluster_global)
					     {
					      cout<<"Cannot open the out_cluster_global file\n"<<endl;
					      exit(0);
					     }
					     
					     
				          out_cluster_global<<endl<<"Cluster status(1-0)/Cluster Rep. point (x,-1)/Core-NonCore(1-0)\n";
					     for(int p=0;p<nrow;p++)
						 {
						   out_cluster_global<<p<<"]";
						   out_cluster_global<<cluster[p].status<<"/";
						   out_cluster_global<<cluster[p].rep<<"/";
						   out_cluster_global<<cluster[p].core;
						   out_cluster_global<<endl;

						 }
						 
				       out_cluster_global.close();
				       
				                                        //Dump clusters binary
									     ofstream cluster_binary("Cluster_global.bin",ios::out|ios::binary);
									     if(!cluster_binary)
									     {
									      cout<<"Cannot open the Cluster_global.bin file\n"<<endl;
									      exit(0);
									     }

									     for(int i=0;i<nrow;i++)
										 {
										    cluster_binary.write((char*)&cluster[i],sizeof(cluster[i]));

										 }

									     cluster_binary.close();
									     
									     
									    
				 //Dump noise points values
   			              ofstream out_noise("noise_points");
					 if(!out_noise)
					    {
					      cout<<"Cannot open the noise_points file\n"<<endl;
					      exit(0);
					    }
					    
					    
				      if(non_core_noise.size() == 0){out_noise<<"";}
				      if(non_core_noise.size() > 0)
				      {

					    for(int i=0;i<non_core_noise.size();i++)
					     {
					       out_noise<<non_core_noise[i]<<endl;	      
					     }
					     					    
				      }				      
				      
				        out_noise.close();
				      
				      				      //Dump noise points binary
									 ofstream noise_binary("noise_points.bin",ios::out|ios::binary);
									    if(!noise_binary)
									    {
									      cout<<"Cannot open the noise_points.bin file\n"<<endl;
									      exit(0);
									    }

									    if(non_core_noise.size() == 0)noise_binary.write((char*)&a,0);
									    if(non_core_noise.size() > 0){
										    for(int i=0;i<non_core_noise.size();i++)
										     {
										       noise_binary.write((char*)&non_core_noise[i],sizeof(non_core_noise[i]));	      
										     }
									       }
									     
									   noise_binary.close();									     
								    				       
				       				       	
				       				       
				  //Dump Row-Column values
				    ofstream out_nrow_ncol("Row_Column");
				      out_nrow_ncol<<nrow<<" "<<ncol;
				      out_nrow_ncol.close();       	 
							  
				    
       
       }//end func.
       
       
 
  //*** FUNCTION     
      //MEMORY FOOTPRINT
	     void process_mem_usage(double& vm_usage, double& resident_set)
		{
			   using std::ios_base;
			   using std::ifstream;
			   using std::string;

			   vm_usage     = 0.0;
			   resident_set = 0.0;

			   // 'file' stat seems to give the most reliable results
			   //
			   ifstream stat_stream("/proc/self/stat",ios_base::in);

			   // dummy vars for leading entries in stat that we don't care about
			   //
			   string pid, comm, state, ppid, pgrp, session, tty_nr;
			   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
			   string utime, stime, cutime, cstime, priority, nice;
			   string O, itrealvalue, starttime;

			   // the two fields we want
			   //
			   unsigned long vsize;
			   long rss;

			   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
				       >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
				       >> utime >> stime >> cutime >> cstime >> priority >> nice
				       >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

			   stat_stream.close();

			   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
			   vm_usage     = vsize / 1024.0;
			   resident_set = rss * page_size_kb;
			   
		}//end func.

       
       


































