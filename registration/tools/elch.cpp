#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/registration/transforms.h>
#include <pcl/registration/icp.h>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
using boost::graph_traits;

#include <boost/graph/adjacency_list.hpp>
using boost::edge_weight_t;

#include <iostream>
using std::cout;
using std::endl;
#include <string>
using std::string;
#include <fstream>
using std::ofstream;

#include <vector>
using std::vector;

#include <list>
using std::list;

#include <algorithm>
using std::swap;

typedef pcl::PointXYZ PointType;
typedef pcl::PointCloud<PointType> Cloud;
typedef Cloud::ConstPtr CloudConstPtr;
typedef Cloud::Ptr CloudPtr;
typedef std::pair<std::string, CloudPtr> CloudPair;
typedef std::vector<CloudPair> CloudVector;

typedef boost::adjacency_list <
    boost::listS, boost::vecS, boost::undirectedS,
    boost::no_property,
    boost::property < edge_weight_t, double > >
  graph_t;

/**
 * graph balancer algorithm computes the weights
 * @param g the graph
 * @param f index of the first node
 * @param l index of the last node
 * @param weights array for the weights
 */
void graph_balancer(graph_t &g, int f, int l, double *weights)
{
  list<int> crossings, branches;
  crossings.push_back(f);
  crossings.push_back(l);
  weights[f] = 0;
  weights[l] = 1;

  int *p = new int[num_vertices(g)];
  int *p_min = new int[num_vertices(g)];
  double *d = new double[num_vertices(g)];
  double *d_min = new double[num_vertices(g)];
  double dist;
  bool do_swap = false;
  list<int>::iterator si, ei, s_min, e_min;

  // process all junctions
  while(!crossings.empty()) {
    dist = -1;
    // find shortest crossing for all vertices on the loop
    for(si = crossings.begin(); si != crossings.end(); ) {
      dijkstra_shortest_paths(g, *si, boost::predecessor_map(p).distance_map(d));
      ei = si;
      ei++;
      // find shortest crossing for one vertex
      for(; ei != crossings.end(); ei++) {
        if(*ei != p[*ei] && (dist < 0 || d[*ei] < dist)) {
          dist = d[*ei];
          s_min = si;
          e_min = ei;
          do_swap = true;
        }
      }
      if(do_swap) {
        swap(p, p_min);
        swap(d, d_min);
        do_swap = false;
      }
      // vertex starts a branch
      if(dist < 0) {
        branches.push_back(*si);
        si = crossings.erase(si);
      } else {
        si++;
      }
    }

    if(dist > -1) {
      remove_edge(*e_min, p_min[*e_min], g);
      for(int i = p_min[*e_min]; i != *s_min; i = p_min[i]) {
        //even right with weights[*s_min] > weights[*e_min]! (math works)
        weights[i] = weights[*s_min] + (weights[*e_min] - weights[*s_min]) * d_min[i] / d_min[*e_min];
        remove_edge(i, p_min[i], g);
        if(degree(i, g) > 0) {
          crossings.push_back(i);
        }
      }

      if(degree(*s_min, g) == 0) {
        crossings.erase(s_min);
      }

      if(degree(*e_min, g) == 0) {
        crossings.erase(e_min);
      }
    }
  }

  delete[] p;
  delete[] p_min;
  delete[] d;
  delete[] d_min;

  graph_traits <graph_t>::adjacency_iterator ai, ai_end;
  int s;

  // error propagation
  while(!branches.empty()) {
    s = branches.front();
    branches.pop_front();

    for(tie(ai, ai_end) = adjacent_vertices(s, g); ai != ai_end; ++ai) {
      weights[*ai] = weights[s];
      if(degree(*ai, g) > 1) {
        branches.push_back(*ai);
      }
    }
    clear_vertex(s, g);
  }
}

void elch(int start, int end, const CloudVector &clouds)
{
  CloudPtr tmp (new Cloud);

  Eigen::Vector4f cstart;
  pcl::compute3DCentroid(*(clouds[start].second), cstart);
  Eigen::Vector3f cs2(cstart[0], cstart[1], cstart[2]);

  Eigen::Vector4f cend2;
  pcl::compute3DCentroid(*(clouds[end].second), cend2);
  Eigen::Vector3f ce2(cend2[0], cend2[1], cend2[2]);
  ce2 = cs2 - ce2;
  pcl::transformPointCloud(*(clouds[end].second), *tmp, ce2, Eigen::Quaternionf::Identity());

  //pcl::IterativeClosestPointNonLinear<PointType, PointType> icp;
  pcl::IterativeClosestPoint<PointType, PointType> icp;

  icp.setMaximumIterations (50);
  icp.setMaxCorrespondenceDistance (1.5);
  icp.setRANSACOutlierRejectionThreshold (1.5);
  //icp.setRANSACOutlierRejectionThreshold (DBL_MAX);

  icp.setInputTarget (clouds[start].second);

  icp.setInputCloud (tmp);

  icp.align (*tmp);

  Eigen::Matrix3f m(icp.getFinalTransformation().block(0, 0, 3, 3));
  Eigen::Quaternionf q(m);
  Eigen::Vector4f ta(icp.getFinalTransformation().col(3));
  Eigen::Vector3f t(ta[0], ta[1], ta[2]);
  //TODO hack
  t += ce2;

  std::cout << icp.getFinalTransformation() << std::endl;

  static graph_t g;

  for(int i = num_vertices(g)+1; i < clouds.size(); i++) {
    add_edge(i-1, i, g);
  }

  graph_t grb[4];

  graph_traits <graph_t>::edge_iterator ei, ei_end;
  for(tie(ei, ei_end) = edges(g); ei != ei_end; ei++) {
    for(int j = 0; j < 4; j++) {
      add_edge(source(*ei, g), target(*ei, g), 1, grb[j]); //TODO add variance
    }
  }

  double *weights[4];
  for(int i = 0; i < 4; i++) {
    weights[i] = new double[clouds.size()];
    graph_balancer(grb[i], start, end, weights[i]);
  }

  //TODO use pose
  Eigen::Vector4f cend;
  pcl::compute3DCentroid(*(clouds[end].second), cend);
  Eigen::Translation3f tend(cend[0], cend[1], cend[2]);
  Eigen::Affine3f aend(tend);
  Eigen::Affine3f aendI = aend.inverse();

  for (int i = 0; i < clouds.size(); i++)
  {
    CloudPtr tmp (new Cloud);

    Eigen::Vector3f t2;
    t2[0] = t[0] * weights[0][i];
    t2[1] = t[1] * weights[1][i];
    t2[2] = t[2] * weights[2][i];

    Eigen::Quaternionf q2;
    q2 = Eigen::Quaternionf::Identity().slerp(weights[3][i], q);

    //TODO use rotation from branch start
    Eigen::Translation3f t3(t2);
    Eigen::Affine3f a(t3 * q2);
    a = aend * a * aendI;
    
    //std::cout << "transform cloud " << i << " to:" << std::endl << a.matrix() << std::endl;
    pcl::transformPointCloud(*(clouds[i].second), *(clouds[i].second), a);
  }
}


void loopDetection(int end, const CloudVector &clouds, double dist)
{
  static double min_dist = -1;
  static int first, last;
  int state = 0;

  for(int i = end-1; i > 0; i--)
  {
    Eigen::Vector4f cstart, cend;
    //TODO use pose of scan
    pcl::compute3DCentroid(*(clouds[i].second), cstart);
    pcl::compute3DCentroid(*(clouds[end].second), cend);
    Eigen::Vector4f diff = cend - cstart;

    double norm = diff.norm();

    //std::cout << "distance between " << i << " and " << end << " is " << norm << " state is " << state << std::endl;

    if(state == 0 && norm > dist)
    {
      state = 1;
      //std::cout << "state 1" << std::endl;
    }
    if(state > 0 && norm < dist)
    {
      state = 2;
      std::cout << "loop detected between scan " << i << " (" << clouds[i].first << ") and scan " << end << " (" << clouds[end].first << ")" << std::endl;
      if(min_dist < 0 || norm < min_dist)
      {
        min_dist = norm;
        first = i;
        last = end;
      }
    }
  }
  std::cout << "min_dist: " << min_dist << " state: " << state << " first: " << first << " end: " << end << std::endl;
  if(min_dist > 0 && (state < 2 || end == clouds.size()-1)) //TODO
  {
    min_dist = -1;
    std::cout << "calling elch with " << first << " and " << last << std::endl;
    elch(first, last, clouds);
    std::cout << "finished calling elch" << std::endl;
  }
}

int
main (int argc, char **argv)
{
  CloudVector clouds;
  for(int i = 1; i < argc; i++) {
    CloudPtr pc (new Cloud);
    pcl::io::loadPCDFile(argv[i], *pc);
    clouds.push_back(CloudPair(argv[i], pc));
    std::cout << "loading file: " << argv[i] << " size: " << pc->size() << std::endl;
  }

  for(int i = 0; i < clouds.size(); i++)
    loopDetection(i, clouds, 15.0);

  for(int i = 0; i < clouds.size(); i++)
  {
    std::string result_filename(clouds[i].first);
    result_filename = result_filename.substr (result_filename.rfind ("/") + 1);
    pcl::io::savePCDFileBinary (result_filename.c_str(), *(clouds[i].second));
    cout << "saving result to " << result_filename << endl;
  }

  return 0;
}
