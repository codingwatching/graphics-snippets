/***********************************************************************************************//**
* \brief GLFW-3 window implementation.
*
* [GLFW](https://www.glfw.org/)
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/

#include <stdafx.h>

// include

#include <glfw3_window.h>


// GLFW-3

#include <GLFW/glfw3.h>


namespace View
{


namespace GLFW3 
{


/***********************************************************************************************//**
* \brief GLFW exception object  
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
class GLFWError
    : public View::Error
{
public:

    GLFWError( 
        const std::string &message ) //! error message
        : View::Error( "GLFW-3: " + message )
    {}
};


/***********************************************************************************************//**
* \brief GLFW-3 window pointer  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
class CWindowHandle
{
public:
    GLFWwindow *_window{ nullptr };

    static CWindow * WindowPtr( GLFWwindow *window )
    {
        void *ptr = glfwGetWindowUserPointer( window );
        return static_cast<CWindow*>(ptr);
    }

    TSize FrabufferSize( void ) const
    {
        int cx, cy;
        glfwGetFramebufferSize(_window, &cx, &cy );
        return { (TScale)cx, (TScale)cy };
    }
};


/***********************************************************************************************//**
* \brief ctor  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
CWindow::CWindow( 
    const std::string &name ) //!< name of the window
    : _name( name )
{
    if ( Valid() )
        glfwDestroyWindow( _handle->_window  );
}


/***********************************************************************************************//**
* \brief dtor  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
CWindow::~CWindow()
{}


/***********************************************************************************************//**
* \brief Validate window handle and throw an exception if the validation fails.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Validate( void ) const
{
    if ( Valid() == false )
        throw GLFWError( "invalid window handle" );
}


/***********************************************************************************************//**
* \brief Initialize the window  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
bool CWindow::Init(
    const TViewSettings &parameters ) //!< initialization parameter
{
    class CInstance
    {
    public:
        CInstance( void )
        {
            if ( glfwInit() == GLFW_FALSE )
                throw GLFWError("error initializing");
        }

        virtual ~CInstance()
        {
            glfwTerminate();
        }
    };

    // initialize GLFW-3
    static CInstance instance;

    // check if window was initialized already 
    if ( Valid() )
        return true;

    _paramters = parameters;

    // [GLFW Window guide; Window creation hints](http://www.glfw.org/docs/latest/window_guide.html#window_hints_values)

    glfwWindowHint( GLFW_DEPTH_BITS, 24 );
    glfwWindowHint( GLFW_STENCIL_BITS, 8 ); 

    glfwWindowHint( GLFW_SAMPLES, _paramters._samples );
    glfwWindowHint( GLFW_DOUBLEBUFFER, _paramters.Test<TCapability::doublebuffer>() ? GLFW_TRUE : GLFW_FALSE );

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    //glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );

    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, _paramters.Test<TCapability::debug>() ? GLFW_TRUE : GLFW_FALSE );

    auto window = glfwCreateWindow( _paramters._size[0], _paramters._size[1], _name.c_str(), nullptr, nullptr );
    if ( window == nullptr )
    {
        glfwTerminate();
        throw GLFWError( "error initializing window" );
    }

    glfwSetWindowUserPointer( window, this );

    glfwSetWindowSizeCallback( window, [](GLFWwindow* window, int cx, int cy)
    {
        if ( CWindow *wndPtr = CWindowHandle::WindowPtr(window) )
            wndPtr->Resize( { (TScale)cx, (TScale)cy } );
    } );

    /*
    
    std::array< int, 2 > _wndPos         {0, 0};
    std::array< int, 2 > _wndSize        {0, 0};
    std::array< int, 2 > _vpSize         {0, 0};
    bool                 _updateViewport = true;
    bool                 _doubleBuffer   = true;
    GLFWwindow *         _wnd            = nullptr;
    GLFWmonitor *        _monitor        = nullptr;

    */

    /*
   
    void CWindow_Glfw::Resize( int cx, int cy )
    {
        _updateViewport = true;
    }
    */

    //_monitor =  glfwGetPrimaryMonitor();
    //glfwGetWindowSize( window, &_wndSize[0], &_wndSize[1] );
    //glfwGetWindowPos( window, &_wndPos[0], &_wndPos[1] );

    _handle = std::make_shared<CWindowHandle>( CWindowHandle( { window } ) );
    _size   = _handle->FrabufferSize();

    // ...

    return Valid();
}


/***********************************************************************************************//**
* \brief   
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
void CWindow::Resize( 
    TSize ) //!< new window size - not this may be different to framebuffer size
{
    _size = _handle->FrabufferSize();
    _state.set( (int)TWindowState::size_changed, true );
}


/***********************************************************************************************//**
* \brief Returns `true` if the widow has received the close notification.
* Otherwise `false` is returned.
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
bool CWindow::Dropped( void ) const
{
    Validate();
    return glfwWindowShouldClose( _handle->_window ) != GLFW_FALSE;
}


/***********************************************************************************************//**
* \brief Evaluate if the size of the window has changed.  
*
* \author  Rabbid76    \date  2019-02-02
***************************************************************************************************/
bool CWindow::SizeChanged( 
    bool reset_changed ) //!< true: rest the size changed state
    noexcept(true)
{
    bool changed = _state.test( (int)TWindowState::size_changed );
    if ( reset_changed )
        _state.reset( (int)TWindowState::size_changed );
    return changed;
}


/***********************************************************************************************//**
* \brief Get the current window size.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
TSize CWindow::Size( void ) const noexcept(true)
{
    return _size;
}


/***********************************************************************************************//**
* \brief Handle pending events.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::HandleEvents( void ) const
{
    Validate();
    glfwPollEvents();
}


/***********************************************************************************************//**
* \brief Make context current.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Activate( void ) const
{
    Validate();
    glfwMakeContextCurrent( _handle->_window );
}


/***********************************************************************************************//**
* \brief Flush or swap the display buffers.  
*
* \author  Rabbid76    \date  2019-02-01
***************************************************************************************************/
void CWindow::Flush( void ) const
{
    Validate();
    glfwSwapBuffers( _handle->_window );
}


} // GLFW3


} // View