#!/usr/bin/env python3
import os
import sys

if __name__ == '__main__':
    base_dir = os.path.dirname(os.path.abspath(__file__))
    html_path = os.path.join(base_dir, 'index.html')
    header_path = os.path.join(base_dir, 'web_page.h')

    if not os.path.exists(html_path):
        print(f"error: {html_path} can't found!")
        sys.exit(1)

    with open(html_path, 'r', encoding='utf-8') as f:
        html_content = f.read()

    header_content = f'''#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
{html_content}
)rawliteral";

#endif
'''

    with open(header_path, 'w', encoding='utf-8') as f:
        f.write(header_content)

    print(f"done")


   
