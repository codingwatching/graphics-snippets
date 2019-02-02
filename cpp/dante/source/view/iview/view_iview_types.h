/***********************************************************************************************//**
* \brief Interfaces related to view, window and display.
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#pragma once
#ifndef __View_IView_Types__h__
#define __View_IView_Types__h__

// include

#include <utility_vector_types.h>


// STL

#include <bitset>
#include <exception> 


namespace View
{

// view exception
using Error = std::runtime_error;


// scale type for view size and point
typedef int TScale; 


//! view size
using TSize = std::array<TScale, 2>;


//! view point
using TPoint = std::array<TScale, 2>;


//! view capability
enum class TCapability
{
    doublebuffer, //!< double buffering
    debug,        //!< debug context

    NO_OF         //!< number of properties  
};

using TCapabilitySet = std::bitset<(int)TCapability::NO_OF>;


//! view initial settings
class TViewSettings
{
public:

    template<TCapability PROPERTY>
    bool Test( void ) const { return _cpability.test((size_t)PROPERTY); }
    template<TCapability PROPERTY>
    void Set( bool falg ) { _cpability.set((size_t)PROPERTY, falg); }

    TSize          _size;      //!< size of the view
    TScale         _samples;   //!< multisamples
    TCapabilitySet _cpability; //!< capability properties
};


} // View

#endif // __View_IView_Types__h__