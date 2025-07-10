import json
import base64
import os
import logging
from typing import Dict, List, Any, Optional, Union, Tuple
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.parse

# 로깅 설정
logging.basicConfig(
    level=logging.INFO, 
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('mcp_server')

# MCP 프로토콜에 따른 응답을 생성하는 클래스
class MCPServer:
    def __init__(self):
        # API 설명을 저장하는 딕셔너리
        self.api_descriptions = {
            "tools": {
                "add": "두 개의 float 숫자를 더합니다.",
                "subtract": "첫 번째 숫자에서 두 번째 숫자를 뺍니다.",
                "multiply": "두 개의 float 숫자를 곱합니다.",
                "divide": "첫 번째 숫자를 두 번째 숫자로 나눕니다."
            },
            "resources": {
                "read_text": "지정된 경로의 텍스트 파일을 읽고 내용을 반환합니다.",
                "read_image": "지정된 경로의 이미지 파일을 읽고 base64로 인코딩된 문자열로 반환합니다."
            },
            "templates": {
                "greeting": "사용자에게 인사말을 제공합니다."
            },
            "prompts": {
                "summarize": "제공된 텍스트를 요약합니다."
            }
        }
    
    def process_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """MCP 요청을 처리하고 적절한 응답을 반환합니다."""
        try:
            if "api" not in request:
                return self.error_response("API not specified")
            
            api = request["api"]
            logger.info(f"Processing request for API: {api}")
            
            if api == "describe":
                return self.describe_api()
            elif api.startswith("tools/"):
                tool_name = api.split("/")[1]
                return self.handle_tools_api(tool_name, request.get("params", {}))
            elif api.startswith("resources/"):
                resource_name = api.split("/")[1]
                return self.handle_resources_api(resource_name, request.get("params", {}))
            elif api.startswith("templates/"):
                template_name = api.split("/")[1]
                return self.handle_templates_api(template_name, request.get("params", {}))
            elif api.startswith("prompts/"):
                prompt_name = api.split("/")[1]
                return self.handle_prompts_api(prompt_name, request.get("params", {}))
            else:
                return self.error_response(f"Unknown API: {api}")
                
        except Exception as e:
            logger.exception("Error processing request")
            return self.error_response(str(e))
    
    def describe_api(self) -> Dict[str, Any]:
        """서버가 제공하는 모든 API에 대한 설명을 반환합니다."""
        return {
            "status": "success",
            "data": {
                "apis": {
                    "describe": "서버가 제공하는 모든 API에 대한 설명을 반환합니다.",
                    "tools/add": self.api_descriptions["tools"]["add"],
                    "tools/subtract": self.api_descriptions["tools"]["subtract"],
                    "tools/multiply": self.api_descriptions["tools"]["multiply"],
                    "tools/divide": self.api_descriptions["tools"]["divide"],
                    "resources/read_text": self.api_descriptions["resources"]["read_text"],
                    "resources/read_image": self.api_descriptions["resources"]["read_image"],
                    "templates/greeting": self.api_descriptions["templates"]["greeting"],
                    "prompts/summarize": self.api_descriptions["prompts"]["summarize"]
                }
            }
        }
    
    def handle_tools_api(self, tool_name: str, params: Dict[str, Any]) -> Dict[str, Any]:
        """도구 API를 처리합니다."""
        if tool_name == "add":
            return self.add_numbers(params)
        elif tool_name == "subtract":
            return self.subtract_numbers(params)
        elif tool_name == "multiply":
            return self.multiply_numbers(params)
        elif tool_name == "divide":
            return self.divide_numbers(params)
        else:
            return self.error_response(f"Unknown tool: {tool_name}")
    
    def add_numbers(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """두 숫자를 더합니다."""
        try:
            a = float(params.get("a", 0))
            b = float(params.get("b", 0))
            result = a + b
            return {
                "status": "success",
                "data": {"result": result}
            }
        except ValueError:
            return self.error_response("Invalid number format")
    
    def subtract_numbers(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """첫 번째 숫자에서 두 번째 숫자를 뺍니다."""
        try:
            a = float(params.get("a", 0))
            b = float(params.get("b", 0))
            result = a - b
            return {
                "status": "success",
                "data": {"result": result}
            }
        except ValueError:
            return self.error_response("Invalid number format")
    
    def multiply_numbers(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """두 숫자를 곱합니다."""
        try:
            a = float(params.get("a", 0))
            b = float(params.get("b", 0))
            result = a * b
            return {
                "status": "success",
                "data": {"result": result}
            }
        except ValueError:
            return self.error_response("Invalid number format")
    
    def divide_numbers(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """첫 번째 숫자를 두 번째 숫자로 나눕니다."""
        try:
            a = float(params.get("a", 0))
            b = float(params.get("b", 0))
            if b == 0:
                return self.error_response("Division by zero")
            result = a / b
            return {
                "status": "success",
                "data": {"result": result}
            }
        except ValueError:
            return self.error_response("Invalid number format")
    
    def handle_resources_api(self, resource_name: str, params: Dict[str, Any]) -> Dict[str, Any]:
        """리소스 API를 처리합니다."""
        if resource_name == "read_text":
            return self.read_text_file(params)
        elif resource_name == "read_image":
            return self.read_image_file(params)
        else:
            return self.error_response(f"Unknown resource: {resource_name}")
    
    def read_text_file(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """지정된 경로의 텍스트 파일을 읽습니다."""
        path = params.get("path")
        if not path:
            return self.error_response("Path parameter is required")
        
        try:
            with open(path, "r", encoding="utf-8") as file:
                content = file.read()
            return {
                "status": "success",
                "data": {"content": content}
            }
        except FileNotFoundError:
            return self.error_response(f"File not found: {path}")
        except Exception as e:
            return self.error_response(f"Error reading file: {str(e)}")
    
    def read_image_file(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """지정된 경로의 이미지 파일을 읽고 base64로 인코딩합니다."""
        path = params.get("path")
        if not path:
            return self.error_response("Path parameter is required")
        
        try:
            with open(path, "rb") as file:
                image_data = file.read()
                
            # 이미지 데이터를 base64로 인코딩
            encoded_data = base64.b64encode(image_data).decode("utf-8")
            
            # 파일 확장자 추출
            _, file_ext = os.path.splitext(path)
            file_ext = file_ext.lower().strip(".")
            
            # MIME 타입 설정
            mime_types = {
                "jpg": "image/jpeg",
                "jpeg": "image/jpeg",
                "png": "image/png",
                "gif": "image/gif",
                "bmp": "image/bmp",
                "webp": "image/webp"
            }
            mime_type = mime_types.get(file_ext, "application/octet-stream")
            
            return {
                "status": "success",
                "data": {
                    "content": encoded_data,
                    "mime_type": mime_type
                }
            }
        except FileNotFoundError:
            return self.error_response(f"File not found: {path}")
        except Exception as e:
            return self.error_response(f"Error reading image: {str(e)}")
    
    def handle_templates_api(self, template_name: str, params: Dict[str, Any]) -> Dict[str, Any]:
        """템플릿 API를 처리합니다."""
        if template_name == "greeting":
            return self.greeting_template(params)
        else:
            return self.error_response(f"Unknown template: {template_name}")
    
    def greeting_template(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """인사말 템플릿을 반환합니다."""
        name = params.get("name", "Guest")
        time_of_day = params.get("time_of_day", "day")
        
        greeting = f"안녕하세요, {name}님! 좋은 {time_of_day} 되세요."
        
        return {
            "status": "success",
            "data": {
                "template": "greeting",
                "content": greeting
            }
        }
    
    def handle_prompts_api(self, prompt_name: str, params: Dict[str, Any]) -> Dict[str, Any]:
        """프롬프트 API를 처리합니다."""
        if prompt_name == "summarize":
            return self.summarize_prompt(params)
        else:
            return self.error_response(f"Unknown prompt: {prompt_name}")
    
    def summarize_prompt(self, params: Dict[str, Any]) -> Dict[str, Any]:
        """요약 프롬프트를 반환합니다."""
        text = params.get("text", "")
        if not text:
            return self.error_response("Text parameter is required")
        
        # 실제로는 AI 모델이 요약을 수행해야 하지만, 여기서는 간단한 예시를 제공합니다.
        prompt = f"""다음 텍스트를 간결하게 요약해주세요:

{text}

요약:
"""
        
        return {
            "status": "success",
            "data": {
                "prompt": prompt
            }
        }
    
    def error_response(self, message: str) -> Dict[str, Any]:
        """오류 응답을 생성합니다."""
        return {
            "status": "error",
            "error": {
                "message": message
            }
        }

# HTTP 핸들러 클래스
class MCPHttpHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.mcp_server = MCPServer()
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        """GET 요청 처리"""
        self._handle_request()
    
    def do_POST(self):
        """POST 요청 처리"""
        self._handle_request()
    
    def _handle_request(self):
        """모든 HTTP 요청을 처리하는 공통 메소드"""
        try:
            # URL 파싱 및 쿼리 파라미터 추출
            parsed_url = urllib.parse.urlparse(self.path)
            path = parsed_url.path
            
            # /mcp 엔드포인트로만 처리
            if not path.startswith('/mcp'):
                self.send_error(404, "Not found")
                return
            
            # 요청 바디 읽기
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length) if content_length > 0 else b'{}'
            
            try:
                # JSON 파싱
                request = json.loads(post_data.decode('utf-8'))
                logger.info(f"Received request: {request}")
                
                # MCP 서버 처리
                response = self.mcp_server.process_request(request)
                
                # 응답 전송
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')  # CORS 지원
                self.end_headers()
                
                response_json = json.dumps(response)
                self.wfile.write(response_json.encode('utf-8'))
                
            except json.JSONDecodeError:
                logger.error("Invalid JSON in request")
                self.send_error(400, "Invalid JSON format")
            
        except Exception as e:
            logger.exception("Error handling request")
            self.send_error(500, f"Internal server error: {str(e)}")

# 스트리밍 가능한 HTTP 서버 클래스
class StreamableHTTPServer(HTTPServer):
    def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True):
        super().__init__(server_address, RequestHandlerClass, bind_and_activate)
        logger.info(f"Server started at http://{server_address[0]}:{server_address[1]}")

def main():
    """메인 함수"""
    # 서버 설정 및 시작
    server_address = ('', 8000)  # 기본 포트 8000
    httpd = StreamableHTTPServer(server_address, MCPHttpHandler)
    
    try:
        logger.info("MCP Server is running on port 8000...")
        httpd.serve_forever()
    except KeyboardInterrupt:
        logger.info("Server shutting down...")
    finally:
        httpd.server_close()
        logger.info("Server closed.")

if __name__ == "__main__":
    main()
