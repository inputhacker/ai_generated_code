from mcp.server.fastmcp import FastMCP
import os
import base64

app = FastMCP()

# 1. float 숫자간 사칙연산 tools api
@app.tools_api('/calculate/add', desc='두 float 숫자를 더합니다.')
def add(a: float, b: float):
    return {'result': a + b}

@app.tools_api('/calculate/subtract', desc='두 float 숫자를 뺍니다.')
def subtract(a: float, b: float):
    return {'result': a - b}

@app.tools_api('/calculate/multiply', desc='두 float 숫자를 곱합니다.')
def multiply(a: float, b: float):
    return {'result': a * b}

@app.tools_api('/calculate/divide', desc='두 float 숫자를 나눕니다. 0으로 나누면 에러가 발생합니다.')
def divide(a: float, b: float):
    if b == 0:
        return {'error': '0으로 나눌 수 없습니다.'}
    return {'result': a / b}

# 3. 지정 path의 txt 파일 읽기
@app.resources_api('/read/text', desc='지정한 경로의 텍스트 파일을 UTF-8로 읽어 반환합니다.')
def read_text(path: str):
    try:
        with open(path, encoding='utf-8') as f:
            content = f.read()
        return {'content': content}
    except Exception as e:
        return {'error': str(e)}

# 4. 지정 path의 이미지 파일 읽어 base64 반환
@app.resources_api('/read/image', desc='지정한 경로의 이미지 파일을 base64로 반환합니다.')
def read_image(path: str):
    try:
        with open(path, 'rb') as f:
            ext = os.path.splitext(path)[-1].lower()
            mime = {
                '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.png': 'image/png',
                '.gif': 'image/gif', '.bmp': 'image/bmp'
            }.get(ext, 'application/octet-stream')
            bdata = base64.b64encode(f.read()).decode('utf-8')
            return {'image_base64': f'data:{mime};base64,{bdata}'}
    except Exception as e:
        return {'error': str(e)}

# 5. 임의의 MCP 리소스 template 예제
@app.resources_api('/template/example', desc='임의의 사용자 정보 템플릿 데이터 예제 반환')
def example_template(user: str, age: int):
    return {
        'template': {
            'user': user,
            'age': age,
            'message': f'{user}님의 나이는 {age}세입니다.'
        }
    }

# 6. MCP 프롬프트형 API 예제
@app.prompt_api('/prompt/echo', desc='prompt와 name을 받아 메시지를 출력합니다.')
def echo_prompt(prompt: str, name: str):
    return {'content': f"{name}님, 입력하신 프롬프트: \"{prompt}\""}

# 2. 모든 API description 제공(mcp protocol 표준)
@app.description_api()
def api_descriptions():
    return app.get_api_description()  # fastmcp가 지원하는 표준 함수

if __name__ == '__main__':
    app.run_stdio()
