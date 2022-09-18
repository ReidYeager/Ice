
#include "defines.h"
#include "math/vector.h"

//=========================
// f32
//=========================

Ice::vec2::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec2::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec2::operator Ice::vec3 () const { return { (f32)x, (f32)y, 0.0f }; }
Ice::vec2::operator Ice::vec3I() const { return { (i32)x, (i32)y, 0    }; }
Ice::vec2::operator Ice::vec3U() const { return { (u32)x, (u32)y, 0    }; }
Ice::vec2::operator Ice::vec4 () const { return { (f32)x, (f32)y, 0.0f, 0.0f }; }
Ice::vec2::operator Ice::vec4I() const { return { (i32)x, (i32)y, 0   , 0    }; }
Ice::vec2::operator Ice::vec4U() const { return { (u32)x, (u32)y, 0   , 0    }; }

Ice::vec3::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec3::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec3::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec3::operator Ice::vec3I() const { return { (i32)x, (i32)y, (i32)z }; }
Ice::vec3::operator Ice::vec3U() const { return { (u32)x, (u32)y, (u32)z }; }
Ice::vec3::operator Ice::vec4 () const { return { (f32)x, (f32)y, (f32)z, 0.0f }; }
Ice::vec3::operator Ice::vec4I() const { return { (i32)x, (i32)y, (i32)z, 0    }; }
Ice::vec3::operator Ice::vec4U() const { return { (u32)x, (u32)y, (u32)z, 0    }; }

Ice::vec4::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec4::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec4::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec4::operator Ice::vec3 () const { return { (f32)x, (f32)y, (f32)z }; }
Ice::vec4::operator Ice::vec3I() const { return { (i32)x, (i32)y, (i32)z }; }
Ice::vec4::operator Ice::vec3U() const { return { (u32)x, (u32)y, (u32)z }; }
Ice::vec4::operator Ice::vec4I() const { return { (i32)x, (i32)y, (i32)z, (i32)w }; }
Ice::vec4::operator Ice::vec4U() const { return { (u32)x, (u32)y, (u32)z, (u32)w }; }

//=========================
// i32
//=========================

Ice::vec2I::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec2I::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec2I::operator Ice::vec3 () const { return { (f32)x, (f32)y, 0.0f }; }
Ice::vec2I::operator Ice::vec3I() const { return { (i32)x, (i32)y, 0    }; }
Ice::vec2I::operator Ice::vec3U() const { return { (u32)x, (u32)y, 0    }; }
Ice::vec2I::operator Ice::vec4 () const { return { (f32)x, (f32)y, 0.0f, 0.0f }; }
Ice::vec2I::operator Ice::vec4I() const { return { (i32)x, (i32)y, 0   , 0    }; }
Ice::vec2I::operator Ice::vec4U() const { return { (u32)x, (u32)y, 0   , 0    }; }

Ice::vec3I::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec3I::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec3I::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec3I::operator Ice::vec3 () const { return { (f32)x, (f32)y, (f32)z }; }
Ice::vec3I::operator Ice::vec3U() const { return { (u32)x, (u32)y, (u32)z }; }
Ice::vec3I::operator Ice::vec4 () const { return { (f32)x, (f32)y, (f32)z, 0.0f }; }
Ice::vec3I::operator Ice::vec4I() const { return { (i32)x, (i32)y, (i32)z, 0    }; }
Ice::vec3I::operator Ice::vec4U() const { return { (u32)x, (u32)y, (u32)z, 0    }; }

Ice::vec4I::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec4I::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec4I::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec4I::operator Ice::vec3 () const { return { (f32)x, (f32)y, (f32)z }; }
Ice::vec4I::operator Ice::vec3I() const { return { (i32)x, (i32)y, (i32)z }; }
Ice::vec4I::operator Ice::vec3U() const { return { (u32)x, (u32)y, (u32)z }; }
Ice::vec4I::operator Ice::vec4 () const { return { (f32)x, (f32)y, (f32)z, (f32)w }; }
Ice::vec4I::operator Ice::vec4U() const { return { (u32)x, (u32)y, (u32)z, (u32)w }; }

//=========================
// u32
//=========================

Ice::vec2U::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec2U::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec2U::operator Ice::vec3 () const { return { (f32)x, (f32)y, 0.0f }; }
Ice::vec2U::operator Ice::vec3I() const { return { (i32)x, (i32)y, 0    }; }
Ice::vec2U::operator Ice::vec3U() const { return { (u32)x, (u32)y, 0    }; }
Ice::vec2U::operator Ice::vec4 () const { return { (f32)x, (f32)y, 0.0f, 0.0f }; }
Ice::vec2U::operator Ice::vec4I() const { return { (i32)x, (i32)y, 0   , 0    }; }
Ice::vec2U::operator Ice::vec4U() const { return { (u32)x, (u32)y, 0   , 0    }; }

Ice::vec3U::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec3U::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec3U::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec3U::operator Ice::vec3 () const { return { (f32)x, (f32)y, (f32)z }; }
Ice::vec3U::operator Ice::vec3I() const { return { (i32)x, (i32)y, (i32)z }; }
Ice::vec3U::operator Ice::vec4 () const { return { (f32)x, (f32)y, (f32)z, 0.0f }; }
Ice::vec3U::operator Ice::vec4I() const { return { (i32)x, (i32)y, (i32)z, 0    }; }
Ice::vec3U::operator Ice::vec4U() const { return { (u32)x, (u32)y, (u32)z, 0    }; }

Ice::vec4U::operator Ice::vec2 () const { return { (f32)x, (f32)y }; }
Ice::vec4U::operator Ice::vec2I() const { return { (i32)x, (i32)y }; }
Ice::vec4U::operator Ice::vec2U() const { return { (u32)x, (u32)y }; }
Ice::vec4U::operator Ice::vec3 () const { return { (f32)x, (f32)y, (f32)z }; }
Ice::vec4U::operator Ice::vec3I() const { return { (i32)x, (i32)y, (i32)z }; }
Ice::vec4U::operator Ice::vec3U() const { return { (u32)x, (u32)y, (u32)z }; }
Ice::vec4U::operator Ice::vec4 () const { return { (f32)x, (f32)y, (f32)z, (f32)w }; }
Ice::vec4U::operator Ice::vec4I() const { return { (i32)x, (i32)y, (i32)z, (i32)w }; }
