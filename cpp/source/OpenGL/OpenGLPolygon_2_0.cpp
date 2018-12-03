/******************************************************************//**
* \brief Implementation of OpenGL polygon renderer,
* with the use of very simple and highly optimized shaders,
* for OpenGL version 2.00 and GLSL version 1.10 (`#version 110`).
* 
* \author  gernot
* \date    2018-08-01
* \version 1.0
**********************************************************************/

#include <stdafx.h>

// includes

#include <OpenGLPolygon_2_0.h>
#include <OpenGLPrimitive_2_0.h>


// OpenGL wrapper

#include <OpenGL_include.h>
#include <OpenGL_enumconst.h>


// STL

#include <iostream>
#include <algorithm>


// preprocessor definitions

#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif


// class definitions


/******************************************************************//**
* \brief General namespace for OpenGL implementation.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
namespace OpenGL
{


/******************************************************************//**
* \brief Namespace for drawing polygons with the use of OpenGL.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
namespace Polygon
{


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPolygonOpenGL_2_00::CPolygonOpenGL_2_00( 
  size_t min_cache_elems ) //! I - size of the element cache
  : _min_cache_elems( min_cache_elems )
  , _elem_cache( min_cache_elems, 0.0f )
{}


/******************************************************************//**
* \brief ctor  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
CPolygonOpenGL_2_00::~CPolygonOpenGL_2_00()
{}


/******************************************************************//**
* \brief Initialize the polygon renderer.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
void CPolygonOpenGL_2_00::Init( void )
{
  if ( _primitive_prog != nullptr )
    return;

  _primitive_prog = std::make_unique<CPrimitiveOpenGL_2_00>();
  _primitive_prog->Init();
}


/******************************************************************//**
* \brief Notify the render that a sequence of successive polygons will
* follow, that is not interrupted by any other drawing operation.
* This allows the render to do some performance optimizations and to
* prepare for the polygon rendering.
* The render can keep states persistent from one polygon drawing to the
* other, without initializing and restoring them.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::StartSuccessivePolygonDrawings( void )
{
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  _primitive_prog->StartSuccessivePrimitiveDrawings();
  return true;
}


/******************************************************************//**
* \brief Notify the renderer that a sequence of polygons has been
* finished, and that the internal states have to be restored.  
* 
* \author  gernot
* \date    2018-09-12
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::FinishSuccessivePolygonDrawings( void )
{
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  _primitive_prog->FinishSuccessivePrimitiveDrawings();
  return true;
}


/******************************************************************//**
* \brief Set the stroke color of the polygon.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_2_00::SetColor(
  const Render::TColor & color ) //!< I - the new color
{
  if ( _primitive_prog != nullptr )
    _primitive_prog->SetColor( color );
  return *this; 
}


/******************************************************************//**
* \brief Set the stroke color of the polygon.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_2_00::SetColor( 
  const Render::TColor8 & color ) //!< I - the new color
{
  if ( _primitive_prog != nullptr )
    _primitive_prog->SetColor( color );
  return *this; 
}


/******************************************************************//**
* \brief Set the polygon style.
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
Render::Polygon::IRender & CPolygonOpenGL_2_00::SetStyle( 
  const Render::Polygon::TStyle & style )
{
  //! This is impossible, while an drawing sequence is active.  
  //! The only possible operations within a `glBegin`/`glEnd` sequence are those operations,
  //! which directly change fixed function attributes or specify a new vertex coordinate.  
  //! See [`glBegin`](https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml)
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    assert( false );
    return *this;
  }
  _primitive_prog->SetDeptAttenuation( _polygon_style._depth_attenuation );

  _polygon_style = style;

  // ...
  
  return *this;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )        //!< in: pointer to an array of the vertex coordinates
{ 
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( false );

  // set vertex attribute pointer and draw the line
  glVertexAttribPointer( prog.Attrib_xyzw_inx(), tuple_size, GL_FLOAT, GL_FALSE, 0, coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
  
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}
 

/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size,     //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
  size_t             coords_size,    //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )        //!< in: pointer to an array of the vertex coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  ASSERT( tuple_size == 2 || tuple_size == 3 || tuple_size == 4 );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  _primitive_prog->ActivateProgram( false );

  // set vertex attribute pointer and draw the line
  glVertexAttribPointer( prog.Attrib_xyzw_inx(), tuple_size, GL_DOUBLE, GL_FALSE, 0, coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)(coords_size / tuple_size) );
 
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const float       *x_coords,       //!< int pointer to an array of the x coordinates
  const float       *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( true );

  // set vertex attribute pointers and draw the line
  glVertexAttribPointer( prog.Attrib_xyzw_inx(), 1, GL_FLOAT, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( prog.Attrib_y_inx(), 1, GL_FLOAT, GL_FALSE, 0, y_coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );
   
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Draw a single polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::Draw( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  size_t             no_of_coords,   //!< in: number of coordinates and number of elements (size) of the coordinate array
  const double      *x_coords,       //!< int pointer to an array of the x coordinates
  const double      *y_coords )      //!< int pointer to an array of the y coordinates
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( true );

  // set vertex attribute pointers and draw the line
  glVertexAttribPointer( prog.Attrib_xyzw_inx(), 1, GL_DOUBLE, GL_FALSE, 0, x_coords );
  glVertexAttribPointer( prog.Attrib_y_inx(), 1, GL_DOUBLE, GL_FALSE, 0, y_coords );
  glDrawArrays( OpenGL::Primitive( primitive_type ), 0, (GLsizei)no_of_coords );

  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();

  return true;
}


/******************************************************************//**
* \brief Start a new polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::StartSequence( 
  Render::TPrimitive primitive_type, //!< in: primitive type of the coordinates - triangles, triangle strip, triangle fan, triangle adjacency or triangle strip adjacency
  unsigned int       tuple_size )    //!< in: kind of the coordinates - 2: 2D (x, y), 3: 3D (x, y, z), 4: homogeneous (x, y, z, w)   
{
  // A new sequence can't be started within an active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() )
  {
    ASSERT( false );
    return false;
  }
  ASSERT( Render::BasePrimitive(primitive_type) == Render::TBasePrimitive::polygon );
  auto &prog = *_primitive_prog;
  
  prog.StartSequence();
  
  _squence_type = primitive_type;
  _tuple_size   = tuple_size;

  return true;
}
  

/******************************************************************//**
* \brief Complete an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::EndSequence( void )
{
  // A sequence can't be completed if there is no active sequence
  if ( _primitive_prog == nullptr || _primitive_prog->EndSequence() == false )
  {
    ASSERT( false );
    return false;
  }
  auto &prog = *_primitive_prog;

  // draw the line

  // activate program, update uniforms and enable vertex attributes
  prog.ActivateProgram( false );

  //  set vertex attribute pointer and draw the line
  glVertexAttribPointer( prog.Attrib_xyzw_inx(), _tuple_size, GL_FLOAT, GL_FALSE, 0, _elem_cache.data() );
  glDrawArrays( OpenGL::Primitive( _squence_type ), 0, (GLsizei)(_sequence_size / _tuple_size) );
  
  // disable vertex attributes and activate program 0
  prog.DeactivateProgram();
  
  _tuple_size    = 0;
  _sequence_size = 0;
  
  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::DrawSequence( 
  float x,  //!< in: x coordinate
  float y,  //!< in: y coordinate
  float z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + _tuple_size )
    _elem_cache.resize( _elem_cache.size() + std::max((size_t)_tuple_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  _elem_cache.data()[_sequence_size++] = x;
  _elem_cache.data()[_sequence_size++] = y;
  if ( _tuple_size >= 3 )
    _elem_cache.data()[_sequence_size++] = z;
  if ( _tuple_size == 4 )
    _elem_cache.data()[_sequence_size++] = 1.0f;

  return true;
}


/******************************************************************//**
* \brief Specify a new vertex coordinate in an active polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::DrawSequence( 
  double x,  //!< in: x coordinate
  double y,  //!< in: y coordinate
  double z ) //!< in: z coordinate
{
  // A sequence has to be active, to specify a new vertex coordinate
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + _tuple_size )
    _elem_cache.resize( _elem_cache.size() + std::max((size_t)_tuple_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  _elem_cache.data()[_sequence_size++] = (float)x;
  _elem_cache.data()[_sequence_size++] = (float)y;
  if ( _tuple_size >= 3 )
    _elem_cache.data()[_sequence_size++] = (float)z;
  if ( _tuple_size == 4 )
    _elem_cache.data()[_sequence_size++] = 1.0f;

  return true;
}
  

/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const float       *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + coords_size )
    _elem_cache.resize( _elem_cache.size() + std::max(coords_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  std::memcpy( _elem_cache.data() + _sequence_size, coords, coords_size * sizeof( float ) );
  _sequence_size += coords_size;
  
  return true;
}


/******************************************************************//**
* \brief Specify a sequence of new vertex coordinates in an active
* polygon sequence.  
* 
* \author  gernot
* \date    2018-12-03
* \version 1.0
**********************************************************************/
bool CPolygonOpenGL_2_00::DrawSequence( 
  size_t             coords_size, //!< in: number of elements (size) of the coordinate array - `coords_size` = `tuple_size` * "number of coordinates" 
  const double      *coords )     //!< in: pointer to an array of the vertex coordinates
{
  // A sequence has to be active, to specify new vertex coordinates
  if ( _primitive_prog == nullptr || _primitive_prog->ActiveSequence() == false )
  {
    ASSERT( false );
    return false;
  }

  // reserve the cache
  if ( _elem_cache.size() < _sequence_size + coords_size )
    _elem_cache.resize( _elem_cache.size() + std::max(coords_size, _min_cache_elems) );

  // add the vertex coordinate to the cache

  float *cache_ptr = _elem_cache.data() + _sequence_size;
  for ( const double *ptr = coords, *end_ptr = coords + coords_size; ptr < end_ptr; ptr ++, cache_ptr ++ )
    *cache_ptr = (float)(*ptr);
  _sequence_size += coords_size;

  return true;
}


} // Polygon


} // OpenGL