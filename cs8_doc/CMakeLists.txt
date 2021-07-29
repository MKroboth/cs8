cmake_minimum_required(VERSION 3.19)
project(cs8_doc)

include(pandocology)

add_document(
        TARGET cs8_doc
        OUTPUT_FILE cs8.pdf
        SOURCES cs8.org
)