//========================================================================
//  File : mcp_tools_bridge.cpp
//  Build: g++ -std=c++17 -O2 mcp_tools_bridge.cpp -o mcp_tools_bridge
//========================================================================
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* -----------------------------------------------------------------------
 *  (1) MCP tools/list  →  Gemini tools list (functionDeclarations)
 *
 *   MCP 예시
 *     {
 *       "tools":[
 *          {
 *            "name":"searchWeather",
 *            "description":"현재 날씨 검색",
 *            "params":[
 *                { "name":"city", "type":"string", "required":true },
 *                { "name":"unit", "type":"string",
 *                  "enum":["metric","imperial"], "required":false }
 *            ]
 *          },
 *          ...
 *       ]
 *     }
 *
 *   Gemini 요구 형식
 *     {
 *       "tools":[
 *         { "functionDeclarations":[ { ... }, { ... } ] }
 *       ]
 *     }
 * ---------------------------------------------------------------------*/
json toGeminiToolsList(const json& mcpList)
{
    if (!mcpList.contains("tools") || !mcpList["tools"].is_array())
        throw std::runtime_error("MCP JSON 에 'tools' 배열이 없습니다.");

    json functionDeclarations = json::array();

    for (const auto& tool : mcpList["tools"])
    {
        json f;
        f["name"]        = tool.at("name");
        f["description"] = tool.value("description", "");

        /* ---------- parameters ---------- */
        json parameters;
        parameters["type"] = "object";

        json properties = json::object();
        json required   = json::array();

        if (tool.contains("params") && tool["params"].is_array())
        {
            for (const auto& p : tool["params"])
            {
                const std::string paramName = p.at("name");
                json prop;
                prop["type"]        = p.value("type", "string");
                prop["description"] = p.value("description", "");

                if (p.contains("enum") && p["enum"].is_array())
                    prop["enum"] = p["enum"];

                properties[paramName] = prop;

                if (p.value("required", false))
                    required.push_back(paramName);
            }
        }
        parameters["properties"] = properties;
        if (!required.empty()) parameters["required"] = required;

        f["parameters"] = parameters;
        functionDeclarations.push_back(std::move(f));
    }

    json gemini;
    gemini["tools"] = json::array();
    gemini["tools"].push_back({ { "functionDeclarations", functionDeclarations } });
    return gemini;
}

/* -----------------------------------------------------------------------
 *  (2) Gemini functionCall  →  MCP tools/call
 *
 *   Gemini 예시
 *     {
 *       "functionCall":{
 *         "name":"searchWeather",
 *         "argsJson":"{\"city\":\"Seoul\",\"unit\":\"metric\"}"
 *       }
 *     }
 *
 *   변환 결과(MCP)
 *     {
 *       "call":{
 *         "name":"searchWeather",
 *         "arguments":{
 *           "city":"Seoul",
 *           "unit":"metric"
 *         }
 *       }
 *     }
 * ---------------------------------------------------------------------*/
json toMcpToolsCall(const json& geminiCall)
{
    if (!geminiCall.contains("functionCall") ||
        !geminiCall["functionCall"].is_object())
        throw std::runtime_error("Gemini JSON 에 'functionCall' 오브젝트가 없습니다.");

    const json& fc = geminiCall["functionCall"];
    std::string   name = fc.at("name").get<std::string>();

    /* argsJson 은 문자열(JSON) → 파싱 */
    json args = json::object();
    if (fc.contains("argsJson"))
    {
        const std::string& argStr = fc.at("argsJson").get_ref<const std::string&>();
        if (!argStr.empty()) args = json::parse(argStr);
    }

    json mcp;
    mcp["call"]["name"]      = name;
    mcp["call"]["arguments"] = args;

    return mcp;
}

/* -----------------------------------------------------------------------
 *  Test Driver (예시용) :
 *    - input_tools_list.json        : MCP tools/list 샘플
 *    - input_tools_call_gemini.json : Gemini functionCall 샘플
 * ---------------------------------------------------------------------*/
int main()
{
    try
    {
        /* 변환 1 : tools/list ----------------------------------------- */
        {
            std::ifstream ifs("input_tools_list.json");
            if (!ifs) throw std::runtime_error("input_tools_list.json 파일을 열 수 없습니다.");
            json mcpList; ifs >> mcpList;

            json geminiList = toGeminiToolsList(mcpList);
            std::cout << "===== Gemini tools list =====\n"
                      << geminiList.dump(2) << "\n\n";
        }

        /* 변환 2 : tools/call ----------------------------------------- */
        {
            std::ifstream ifs("input_tools_call_gemini.json");
            if (!ifs) throw std::runtime_error("input_tools_call_gemini.json 파일을 열 수 없습니다.");
            json geminiCall; ifs >> geminiCall;

            json mcpCall = toMcpToolsCall(geminiCall);
            std::cout << "===== MCP tools/call request =====\n"
                      << mcpCall.dump(2) << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "오류: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
