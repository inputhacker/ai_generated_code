//  File : mcp_to_gemini.cpp
//  빌드 : g++ -std=c++17 -O2 mcp_to_gemini.cpp -o mcp_to_gemini
#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* ---------------------------------------------------------------------------
 * 1) MCP tools/list  →  Gemini tools (functionDeclarations)
 * -------------------------------------------------------------------------*/
json toGeminiTools(const json& mcpToolsList)
{
    if (!mcpToolsList.contains("tools") || !mcpToolsList["tools"].is_array())
        throw std::runtime_error("MCP JSON 에 'tools' 배열이 없습니다.");

    json functionDeclarations = json::array();

    for (const auto& tool : mcpToolsList["tools"])
    {
        json func;
        func["name"]        = tool.at("name");
        func["description"] = tool.value("description", "");

        /* 파라미터 객체 구성 ------------------------------------------- */
        json parameters;
        parameters["type"] = "object";

        json properties = json::object();
        json required   = json::array();

        if (tool.contains("params") && tool["params"].is_array())
        {
            for (const auto& p : tool["params"])
            {
                json property;
                property["type"]        = p.value("type", "string");
                property["description"] = p.value("description", "");

                if (p.contains("enum") && p["enum"].is_array())
                    property["enum"] = p["enum"];

                properties[p.at("name")] = property;

                if (p.value("required", false))
                    required.push_back(p.at("name"));
            }
        }
        parameters["properties"] = properties;
        if (!required.empty())   parameters["required"] = required;

        func["parameters"] = parameters;
        functionDeclarations.push_back(std::move(func));
    }

    /* Gemini 래핑 ------------------------------------------------------- */
    json gemini;
    gemini["tools"] = json::array();
    gemini["tools"].push_back({ { "functionDeclarations", functionDeclarations } });
    return gemini;
}

/* ---------------------------------------------------------------------------
 * 2) MCP tools/call  →  Gemini functionCall
 *    예시 MCP 형식
 *      {
 *        "call": {
 *            "name": "searchWeather",
 *            "arguments": {
 *                "city": "Seoul",
 *                "unit": "metric"
 *            }
 *        }
 *      }
 *
 *    Gemini 형식
 *      {
 *        "functionCall": {
 *            "name": "searchWeather",
 *            "argsJson": "{\"city\":\"Seoul\",\"unit\":\"metric\"}"
 *        }
 *      }
 * -------------------------------------------------------------------------*/
json toGeminiFunctionCall(const json& mcpCall)
{
    if (!mcpCall.contains("call") || !mcpCall["call"].is_object())
        throw std::runtime_error("MCP JSON 에 'call' 오브젝트가 없습니다.");

    const json& callObj = mcpCall["call"];

    std::string name = callObj.at("name").get<std::string>();
    json        args = callObj.value("arguments", json::object());

    json gemini;
    gemini["functionCall"]["name"]     = name;
    gemini["functionCall"]["argsJson"] = args.dump();   // Gemini 요구사항: 문자열 JSON

    return gemini;
}

/* ---------------------------------------------------------------------------
 *  테스트용 메인 :  input_list.json, input_call.json 읽어 변환 결과 출력
 * -------------------------------------------------------------------------*/
int main()
{
    try
    {
        /* tools/list 변환 ---------------------------------------------- */
        {
            std::ifstream ifs("input_list.json");
            if (!ifs) throw std::runtime_error("input_list.json 파일을 열 수 없습니다.");
            json mcpList;  ifs >> mcpList;

            json geminiTools = toGeminiTools(mcpList);
            std::cout << "===== Gemini tools (functionDeclarations) =====\n";
            std::cout << geminiTools.dump(2) << "\n\n";
        }

        /* tools/call 변환 ---------------------------------------------- */
        {
            std::ifstream ifs("input_call.json");
            if (!ifs) throw std::runtime_error("input_call.json 파일을 열 수 없습니다.");
            json mcpCall;  ifs >> mcpCall;

            json geminiCall = toGeminiFunctionCall(mcpCall);
            std::cout << "===== Gemini functionCall =====================\n";
            std::cout << geminiCall.dump(2) << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "오류: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
