# To visit the website of the Vecs2Pauli software, go to [https://vecs2pauli.github.io](https://vecs2pauli.github.io).


Compiling the online solver on the vecs2pauli website
-----------------------------------------------------

Clone this repository (with `--recursive`!):
```
git clone --recursive [url-to-clone-this-repository]
```
The online solver is a WebAssembly file, which we compile using [Emscripten](https://emscripten.org/).
To do so, run in the main folder:

```
emcc -lembind -o website.js website.cpp -I _extern/vecs2pauli/include _extern/vecs2pauli/src/vecs2pauli.cpp -s "EXPORTED_FUNCTIONS=['_malloc']"
```

Then, to check if the website works:

```
python3 -m http.server
```
and open the URL that the command returns in your web browser (typically `http://0.0.0.0:8000/`).
