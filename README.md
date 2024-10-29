# So-Connect
Predict layer-wise connections in social networks

Steps to run the codes related to the project.

a) Initially process the P X C matrix as a (n x 5) dimension dataset.

b) After that prepare the "sdb_pred.cpp" code is prepared for execution.

c) The SNN, KNN, DENSITY values are taken as command line arguments to the "sdb_pred.cpp" code.

d) The **<sdb_pred.cpp>** code is executed thereafter. The clustering results which are obtained are fed to the **<cluster-details.cpp>** code.

e) The **<cluster-details.cpp>** is executed with Cluster-global.bin file as input. Once the code is run, the clusters, outliers, layers and all other necessary information are produced.
