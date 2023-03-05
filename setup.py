from setuptools import Extension, setup

ext = Extension(
    name='glcontext_emscriptem',
    sources=['./glcontext_emscriptem.cpp'],
    define_macros=[('PY_SSIZE_T_CLEAN', None)],
    include_dirs=[],
    library_dirs=[],
    libraries=[],
)

setup(
    name='glcontext_emscriptem',
    version='0.1.0',
    ext_modules=[ext],
)
