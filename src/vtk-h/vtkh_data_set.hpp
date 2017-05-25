#ifndef VTK_H_DATA_SET_HPP
#define VTK_H_DATA_SET_HPP


#include <vector>
#include <string>

#include <vtkh.hpp>
#include <vtkm/cont/DataSet.h>

namespace vtkh
{

class vtkhDataSet
{
public:
  // does it make sense to allow anytype of mesh??
  // for example one domain could be uniform and another
  // could be rectilinear, and one be explicit. 
  // What does that make me?
  std::vector<vtkm::cont::DataSet> m_domains;
  std::vector<int>                 m_domain_ids;
  
  vtkm::Bounds GetBounds() const;
  vtkm::cont::ArrayHandle<vtkm::Range> GetRange(const std::string &field_name) const;
};

} // namespace vtkh

#endif
