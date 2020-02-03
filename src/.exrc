if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
imap <Nul> <C-Space>
inoremap <expr> <Up> pumvisible() ? "\" : "\<Up>"
inoremap <expr> <S-Tab> pumvisible() ? "\" : "\<S-Tab>"
inoremap <expr> <Down> pumvisible() ? "\" : "\<Down>"
inoremap <silent> <SNR>54_AutoPairsReturn =AutoPairsReturn()
inoremap <silent> <Plug>NERDCommenterInsert  <BS>:call NERDComment('i', "insert")
inoremap <silent> <C-Tab> =UltiSnips#ListSnippets()
snoremap <silent>  c
nmap  :call SearchWord()
snoremap  "_c
nmap   @:
nnoremap 'd :YcmShowDetailedDiagnostic
map 'f <Plug>(ctrlp)
nmap 'ca <Plug>NERDCommenterAltDelims
xmap 'cu <Plug>NERDCommenterUncomment
nmap 'cu <Plug>NERDCommenterUncomment
xmap 'cb <Plug>NERDCommenterAlignBoth
nmap 'cb <Plug>NERDCommenterAlignBoth
xmap 'cl <Plug>NERDCommenterAlignLeft
nmap 'cl <Plug>NERDCommenterAlignLeft
nmap 'cA <Plug>NERDCommenterAppend
xmap 'cy <Plug>NERDCommenterYank
nmap 'cy <Plug>NERDCommenterYank
xmap 'cs <Plug>NERDCommenterSexy
nmap 'cs <Plug>NERDCommenterSexy
xmap 'ci <Plug>NERDCommenterInvert
nmap 'ci <Plug>NERDCommenterInvert
nmap 'c$ <Plug>NERDCommenterToEOL
xmap 'cn <Plug>NERDCommenterNested
nmap 'cn <Plug>NERDCommenterNested
xmap 'cm <Plug>NERDCommenterMinimal
nmap 'cm <Plug>NERDCommenterMinimal
xmap 'c  <Plug>NERDCommenterToggle
nmap 'c  <Plug>NERDCommenterToggle
xmap 'cc <Plug>NERDCommenterComment
nmap 'cc <Plug>NERDCommenterComment
xnoremap <silent> '	 :call UltiSnips#SaveLastVisualSelection()gvs
snoremap <silent> '	 :call UltiSnips#ExpandSnippetOrJump()
nnoremap 's :Ag --ignore tags 
nmap 't :CtrlPTag
nmap 'b :CtrlPMRUFiles
nmap 'p "+p
vnoremap 'y "+y
noremap ; :
nmap K :Man =expand('<cword>')
nnoremap \ea :call setline('.', getline('.') . ';')
nnoremap \ed :call setline('.', getline('.')[:-2])
nnoremap co :copen
vmap gx <Plug>NetrwBrowseXVis
nmap gx <Plug>NetrwBrowseX
nnoremap mk :make -j4 
nnoremap mc :make clean 
nnoremap mm :make 
nnoremap <silent> tg :!ctags -R -f .tags
nmap wc :+quit
nnoremap wl l
nnoremap wk k
nnoremap wj j
nnoremap wh h
nnoremap <SNR>97_: :=v:count ? v:count : ''
vnoremap <silent> <Plug>NetrwBrowseXVis :call netrw#BrowseXVis()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#BrowseX(expand((exists("g:netrw_gx")? g:netrw_gx : '<cfile>')),netrw#CheckIfRemote())
nnoremap <silent> <Plug>(ctrlp) :CtrlP
xnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment("x", "Uncomment")
nnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment("n", "Uncomment")
xnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment("x", "AlignBoth")
nnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment("n", "AlignBoth")
xnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment("x", "AlignLeft")
nnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment("n", "AlignLeft")
nnoremap <silent> <Plug>NERDCommenterAppend :call NERDComment("n", "Append")
xnoremap <silent> <Plug>NERDCommenterYank :call NERDComment("x", "Yank")
nnoremap <silent> <Plug>NERDCommenterYank :call NERDComment("n", "Yank")
xnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment("x", "Sexy")
nnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment("n", "Sexy")
xnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment("x", "Invert")
nnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment("n", "Invert")
nnoremap <silent> <Plug>NERDCommenterToEOL :call NERDComment("n", "ToEOL")
xnoremap <silent> <Plug>NERDCommenterNested :call NERDComment("x", "Nested")
nnoremap <silent> <Plug>NERDCommenterNested :call NERDComment("n", "Nested")
xnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment("x", "Minimal")
nnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment("n", "Minimal")
xnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment("x", "Toggle")
nnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment("n", "Toggle")
xnoremap <silent> <Plug>NERDCommenterComment :call NERDComment("x", "Comment")
nnoremap <silent> <Plug>NERDCommenterComment :call NERDComment("n", "Comment")
snoremap <silent> <Del> c
snoremap <silent> <BS> c
snoremap <silent> <C-Tab> :call UltiSnips#ListSnippets()
map <silent> <F6> :TagbarToggle
map <silent> <F5> :NERDTreeToggle
map <silent> <F4> :nohlsearch
inoremap <expr> 	 pumvisible() ? "\" : "\	"
imap  <Plug>DiscretionaryEnd
inoremap <silent> '	 =UltiSnips#ExpandSnippetOrJump()
cnoremap RC :e $MYVIMRC
inoremap \ea :call setline('.', getline('.') . ';')a
inoremap \ed :call setline('.', getline('.')[:-2])a
cnoremap bb :b#
let &cpo=s:cpo_save
unlet s:cpo_save
set autoread
set autowrite
set backspace=eol,start,indent
set complete=.,w,b,u,t,i,k
set completefunc=youcompleteme#CompleteFunc
set completeopt=preview,menuone
set cpoptions=aAceFsB
set fileencodings=utf-8,gb2312,gbk,gb18030
set helplang=en
set hidden
set history=40
set hlsearch
set ignorecase
set incsearch
set laststatus=2
set lazyredraw
set mouse=nv
set path=./,./include/,../,/usr/include/,/usr/include/i386-linux-gnu/,/usr/local/include/,/
set printoptions=paper:a4
set ruler
set runtimepath=~/.vim,~/.vim/plugged/ultisnips/,~/.vim/plugged/vim-snippets/,~/.vim/plugged/rainbow/,~/.vim/plugged/nerdcommenter/,~/.vim/plugged/nerdtree/,~/.vim/plugged/ag/,~/.vim/plugged/tagbar/,~/.vim/plugged/ctrlp.vim/,~/.vim/plugged/vim-airline/,~/.vim/plugged/syntastic/,~/.vim/plugged/youcompleteme/,~/.vim/plugged/vim-asciidoc/,~/.vim/plugged/vim-erlang-runtime/,~/.vim/plugged/vim-erlang-skeletons/,~/.vim/plugged/vim-erlang-omnicomplete/,~/.vim/plugged/vim-erlang-compiler/,~/.vim/plugged/vim-ruby/,~/.vim/plugged/a.vim/,~/.vim/plugged/vim-cpp-enhanced-highlight/,~/.vim/plugged/vim-easymotion/,~/.vim/plugged/vim-fugitive/,~/.vim/plugged/auto-pairs/,~/.vim/plugged/vim-endwise/,~/.vim/plugged/vim-autotag/,~/.vim/plugged/taglist.vim/,~/.vim/plugged/vim-dlang/,~/.vim/plugged/Dutyl/,~/.vim/plugged/vim-colorschemes/,~/.vim/plugged/DoxygenToolkit.vim/,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim80,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/plugged/ultisnips/after,~/.vim/plugged/vim-cpp-enhanced-highlight/after,~/.vim/after
set shortmess=filnxtToOc
set smartcase
set splitbelow
set splitright
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set tags=./.tags;,.tags
set termencoding=utf-8
set viminfo='20,\"100,:20,n~/.viminfo
set whichwrap=b,s,<,>
set wildignore=*.o,*.obj,*.beam,*/.git/*,*/.hg/*,*/.svn/*
set wildmenu
set wildmode=longest:full,full
set nowrapscan
" vim: set ft=vim :
