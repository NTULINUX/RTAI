// ====================================================================
// Copyright (C) 2008 Allan CORNET
// Copyright (C) 2008 Simon LIPP
// Copyright (C) 2008 INRIA
// This file is released into the public domain
// ====================================================================

src_dir = get_absolute_file_path('builder_src.sce');

// tbx_builder_src_lang('fortran', src_dir);
tbx_builder_src_lang('c', src_dir);

clear tbx_builder_src_lang;
clear src_dir;
