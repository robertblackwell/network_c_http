<?php
function this_dir()
{
    return dirname(__FILE__);
}
class GenerateVector
{
    public $interface_string;
    public $impl_string;
    function __construct($file_prefix)
    {
        $this->interface_string = file_get_contents(this_dir() ."/mytype_list.h");
        $this->impl_string =  file_get_contents(this_dir()."/mytype_list.c");
        $this->vector_interface = file_get_contents(this_dir()."/vector.h");
        $this->vector_impl = file_get_contents(this_dir()."/vector.c");
    }
    function vector_h_substitute($type)
    {

    }
    function vector_c_substitute($type)
    {
        
    }
}