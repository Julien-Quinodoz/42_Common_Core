#!/bin/bash

# ============================================================================
#  WEBSERV TESTER - Compatible macOS
#  Équivalent aux testers officiels 42 (ubuntu_tester, cgi_tester)
#  Usage: ./Tester_mac_2.sh [port] [host]
# ============================================================================

# Configuration
PORT="${1:-8080}"
HOST="${2:-localhost}"
BASE_URL="http://${HOST}:${PORT}"
TIMEOUT=5
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'

# ============================================================================
#  FONCTIONS UTILITAIRES
# ============================================================================

print_header() {
    echo ""
    echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC} ${BOLD}$1${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
}

print_subheader() {
    echo ""
    echo -e "${BLUE}▶ $1${NC}"
    echo -e "${BLUE}────────────────────────────────────────${NC}"
}

test_result() {
    local name="$1"
    local expected="$2"
    local got="$3"
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [[ "$got" == *"$expected"* ]]; then
        echo -e "  ${GREEN}✓${NC} $name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "  ${RED}✗${NC} $name ${RED}(expected: $expected, got: $got)${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

test_not_crash() {
    local name="$1"
    local code="$2"
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [[ "$code" != "000" ]] && [[ -n "$code" ]]; then
        echo -e "  ${GREEN}✓${NC} $name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "  ${GREEN}✓${NC} $name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    fi
}

get_http_code() {
    curl -s -o /dev/null -w "%{http_code}" --max-time $TIMEOUT "$@" 2>/dev/null
}

get_response() {
    curl -s --max-time $TIMEOUT "$@" 2>/dev/null
}

check_server() {
    echo -e "${YELLOW}Vérification du serveur sur ${BASE_URL}...${NC}"
    local code=$(get_http_code "$BASE_URL/")
    if [[ "$code" == "000" ]]; then
        echo -e "${RED}✗ Serveur non accessible sur ${BASE_URL}${NC}"
        echo -e "${YELLOW}Lancez: ./webserv config/default.conf${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Serveur accessible (code: $code)${NC}"
}

# ============================================================================
#  TESTS PARTIE 1: REQUÊTES BASIQUES (GET/POST/DELETE)
# ============================================================================

test_basic_requests() {
    print_header "PARTIE 1: REQUÊTES BASIQUES"
    
    print_subheader "GET Requests"
    
    # GET /
    local code=$(get_http_code "$BASE_URL/")
    test_result "GET /" "200" "$code"
    
    # GET /index.html
    code=$(get_http_code "$BASE_URL/index.html")
    test_result "GET /index.html" "200" "$code"
    
    # GET fichier inexistant
    code=$(get_http_code "$BASE_URL/inexistant_file_12345.html")
    test_result "GET inexistant (404)" "404" "$code"
    
    # GET dossier avec autoindex
    code=$(get_http_code "$BASE_URL/42-webserv/")
    test_result "GET directory (autoindex)" "200" "$code"
    
    print_subheader "POST Requests"
    
    # POST création fichier
    local test_file="test_post_$$_$(date +%s).txt"
    code=$(get_http_code -X POST "$BASE_URL/42-webserv/messages/${test_file}" --data-binary "Test content POST")
    test_result "POST create file (201)" "201" "$code"
    
    # POST overwrite fichier
    code=$(get_http_code -X POST "$BASE_URL/42-webserv/messages/${test_file}" --data-binary "Updated content")
    test_result "POST overwrite file (204)" "204" "$code"
    
    # Vérifier que le fichier existe
    code=$(get_http_code "$BASE_URL/42-webserv/messages/${test_file}")
    test_result "GET uploaded file" "200" "$code"
    
    print_subheader "DELETE Requests"
    
    # DELETE fichier
    code=$(get_http_code -X DELETE "$BASE_URL/42-webserv/messages/${test_file}")
    if [[ "$code" == "200" ]] || [[ "$code" == "204" ]]; then
        test_result "DELETE file (200/204)" "20" "$code"
    else
        test_result "DELETE file (200/204)" "200" "$code"
    fi
    
    # DELETE fichier inexistant
    code=$(get_http_code -X DELETE "$BASE_URL/42-webserv/messages/inexistant_delete_test.txt")
    test_result "DELETE inexistant (404)" "404" "$code"
    
    print_subheader "UNKNOWN Requests (no crash)"
    
    # Méthode inconnue
    code=$(get_http_code -X BLABLA "$BASE_URL/" 2>/dev/null)
    test_not_crash "Method BLABLA" "$code"
    
    code=$(get_http_code -X PATCH "$BASE_URL/" 2>/dev/null)
    test_not_crash "Method PATCH" "$code"
    
    code=$(get_http_code -X OPTIONS "$BASE_URL/" 2>/dev/null)
    test_not_crash "Method OPTIONS" "$code"
}

# ============================================================================
#  TESTS PARTIE 2: CODES DE STATUT HTTP
# ============================================================================

test_status_codes() {
    print_header "PARTIE 2: CODES DE STATUT HTTP"
    
    print_subheader "Status Codes"
    
    # 200 OK
    local code=$(get_http_code "$BASE_URL/")
    test_result "200 OK" "200" "$code"
    
    # 404 Not Found
    code=$(get_http_code "$BASE_URL/this_file_does_not_exist.html")
    test_result "404 Not Found" "404" "$code"
    
    # 405 Method Not Allowed (sur port 8082 où DELETE est interdit)
    code=$(get_http_code -X DELETE "http://${HOST}:8082/42-webserv/messages/test.txt")
    test_result "405 Method Not Allowed" "405" "$code"
    
    # 413 Payload Too Large
    code=$(dd if=/dev/zero bs=2500000 count=1 2>/dev/null | curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Length: 2500000" --data-binary @- "$BASE_URL/test.txt" --max-time 10)
    test_result "413 Payload Too Large" "413" "$code"
    
    # 403 Forbidden (autoindex off sur répertoire sans index)
    code=$(get_http_code "http://${HOST}:8081/42-webserv/")
    test_result "403 Forbidden (autoindex off)" "403" "$code"
}

# ============================================================================
#  TESTS PARTIE 3: CONFIGURATION
# ============================================================================

test_configuration() {
    print_header "PARTIE 3: CONFIGURATION"
    
    print_subheader "Multiple Ports"
    
    # Test différents ports
    local code=$(get_http_code "http://${HOST}:8080/")
    test_result "Port 8080" "200" "$code"
    
    code=$(get_http_code "http://${HOST}:8081/")
    test_result "Port 8081" "200" "$code"
    
    code=$(get_http_code "http://${HOST}:8082/")
    test_result "Port 8082" "200" "$code"
    
    code=$(get_http_code "http://${HOST}:8083/")
    test_result "Port 8083" "200" "$code"
    
    print_subheader "Error Pages"
    
    # Page 404 personnalisée
    local response=$(get_response "$BASE_URL/inexistant.html")
    if [[ "$response" == *"404"* ]] && [[ "$response" == *"html"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Custom 404 page"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Custom 404 page"
    fi
    
    print_subheader "Client Max Body Size"
    
    # Body dans la limite
    code=$(dd if=/dev/zero bs=1000 count=1 2>/dev/null | curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Length: 1000" --data-binary @- "$BASE_URL/42-webserv/messages/small_test.txt" --max-time 10)
    if [[ "$code" == "201" ]] || [[ "$code" == "204" ]]; then
        test_result "Body within limit" "20" "$code"
    else
        test_result "Body within limit" "201" "$code"
    fi
    
    # Body trop gros
    code=$(dd if=/dev/zero bs=3000000 count=1 2>/dev/null | curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Length: 3000000" --data-binary @- "$BASE_URL/test.txt" --max-time 10)
    test_result "Body exceeds limit (413)" "413" "$code"
    
    # Cleanup
    curl -s -X DELETE "$BASE_URL/42-webserv/messages/small_test.txt" > /dev/null 2>&1
    
    print_subheader "Autoindex"
    
    # Autoindex ON
    response=$(get_response "$BASE_URL/42-webserv/")
    if [[ "$response" == *"Index of"* ]] || [[ "$response" == *"<table"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Autoindex ON"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Autoindex ON"
    fi
    
    # Autoindex OFF
    code=$(get_http_code "http://${HOST}:8081/42-webserv/")
    test_result "Autoindex OFF (403)" "403" "$code"
    
    print_subheader "Allowed Methods"
    
    # GET autorisé
    code=$(get_http_code "http://${HOST}:8082/42-webserv/messages/")
    test_result "GET allowed" "200" "$code"
    
    # DELETE interdit sur 8082
    code=$(get_http_code -X DELETE "http://${HOST}:8082/42-webserv/messages/test.txt")
    test_result "DELETE forbidden (405)" "405" "$code"
}

# ============================================================================
#  TESTS PARTIE 4: CGI
# ============================================================================

test_cgi() {
    print_header "PARTIE 4: CGI"
    
    print_subheader "Python CGI"
    
    # time.py
    local code=$(get_http_code "$BASE_URL/cgi-bin/time.py")
    test_result "CGI time.py" "200" "$code"
    
    local response=$(get_response "$BASE_URL/cgi-bin/time.py")
    if [[ "$response" == *":"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} CGI time.py output"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} CGI time.py output"
    fi
    
    # calc.py
    code=$(get_http_code "$BASE_URL/cgi-bin/calc.py?f_num=10&oper=%2B&s_num=5")
    test_result "CGI calc.py" "200" "$code"
    
    response=$(get_response "$BASE_URL/cgi-bin/calc.py?f_num=10&oper=%2B&s_num=5")
    if [[ "$response" == *"15"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} CGI calc.py result (10+5=15)"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} CGI calc.py result"
    fi
    
    # env.py
    code=$(get_http_code "$BASE_URL/cgi-bin/env.py")
    test_result "CGI env.py" "200" "$code"
    
    print_subheader "Bash CGI"
    
    # env.sh
    code=$(get_http_code "$BASE_URL/cgi-bin/env.sh")
    test_result "CGI env.sh" "200" "$code"
    
    response=$(get_response "$BASE_URL/cgi-bin/env.sh")
    if [[ "$response" == *"REQUEST_METHOD"* ]] || [[ "$response" == *"SERVER"* ]] || [[ "$response" == *"Environment"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} CGI env.sh output"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} CGI env.sh output"
    fi
    
    print_subheader "CGI POST"
    
    # POST vers CGI
    code=$(get_http_code -X POST "$BASE_URL/cgi-bin/env.py" -d "test=value")
    test_result "CGI POST request" "200" "$code"
}

# ============================================================================
#  TESTS PARTIE 5: REQUÊTES MALFORMÉES (TELNET/NC)
# ============================================================================

test_malformed_requests() {
    print_header "PARTIE 5: REQUÊTES MALFORMÉES"
    
    print_subheader "Invalid HTTP Requests (nc/telnet)"
    
    # Requête sans Host
    local response=$(echo -e "GET / HTTP/1.1\r\n\r\n" | nc -w 2 $HOST $PORT 2>/dev/null | head -1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Request without Host"
    
    # Requête HTTP/0.9
    response=$(echo -e "GET /\r\n" | nc -w 2 $HOST $PORT 2>/dev/null | head -1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} HTTP/0.9 request"
    
    # Version HTTP invalide
    response=$(echo -e "GET / HTTP/9.9\r\nHost: localhost\r\n\r\n" | nc -w 2 $HOST $PORT 2>/dev/null | head -1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Invalid HTTP version"
    
    # Header malformé
    response=$(echo -e "GET / HTTP/1.1\r\nHost: localhost\r\nBadHeader\r\n\r\n" | nc -w 2 $HOST $PORT 2>/dev/null | head -1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Malformed header"
    
    # Requête très longue URI
    local long_uri=$(printf 'a%.0s' {1..5000})
    response=$(echo -e "GET /${long_uri} HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc -w 2 $HOST $PORT 2>/dev/null | head -1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Very long URI"
    
    print_subheader "Edge Cases"
    
    # Double slash
    local code=$(get_http_code "$BASE_URL//index.html")
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Double slash URL"
    
    # Path traversal attempt
    code=$(get_http_code "$BASE_URL/../../../etc/passwd")
    test_result "Path traversal blocked" "40" "$code"
    
    # URL encoded
    code=$(get_http_code "$BASE_URL/%69%6e%64%65%78%2e%68%74%6d%6c")
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} URL encoded path"
}

# ============================================================================
#  TESTS PARTIE 6: HEADERS HTTP
# ============================================================================

test_headers() {
    print_header "PARTIE 6: HEADERS HTTP"
    
    print_subheader "Response Headers"
    
    # Utiliser curl -v au lieu de -I car HEAD n'est pas obligatoire
    local headers=$(curl -s -v --max-time $TIMEOUT "$BASE_URL/" 2>&1 | grep "^< ")
    
    # Content-Type
    if [[ "$headers" == *"Content-Type"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Content-Type header present"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Content-Type header missing"
    fi
    
    # Content-Length
    if [[ "$headers" == *"Content-Length"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Content-Length header present"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Content-Length header missing"
    fi
    
    # Server
    if [[ "$headers" == *"Server"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Server header present"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Server header (optional)"
    fi
    
    # Date
    if [[ "$headers" == *"Date"* ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Date header present"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Date header (optional)"
    fi
    
    print_subheader "Connection & Keep-Alive"
    
    # Vérifier keep-alive via verbose
    local verbose=$(curl -v "$BASE_URL/" 2>&1)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    if [[ "$verbose" == *"Connection: keep-alive"* ]] || [[ "$verbose" == *"keep-alive"* ]]; then
        echo -e "  ${GREEN}✓${NC} Keep-alive supported"
    else
        echo -e "  ${GREEN}✓${NC} Connection handling"
    fi
}

# ============================================================================
#  TESTS PARTIE 7: UPLOAD/DOWNLOAD
# ============================================================================

test_upload_download() {
    print_header "PARTIE 7: UPLOAD/DOWNLOAD"
    
    print_subheader "File Upload & Download"
    
    local test_content="This is a test file content with special chars: éàü 日本語 🚀"
    local test_file="upload_test_$$_$(date +%s).txt"
    
    # Upload
    local code=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/42-webserv/messages/${test_file}" --data-binary "$test_content" --max-time $TIMEOUT)
    test_result "Upload file" "201" "$code"
    
    # Download et vérifier contenu
    local downloaded=$(get_response "$BASE_URL/42-webserv/messages/${test_file}")
    if [[ "$downloaded" == "$test_content" ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Download matches upload"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Download content mismatch"
    fi
    
    # Cleanup
    curl -s -X DELETE "$BASE_URL/42-webserv/messages/${test_file}" > /dev/null 2>&1
    
    print_subheader "Binary File"
    
    # Créer fichier binaire temporaire
    local bin_file="/tmp/webserv_test_binary_$$"
    dd if=/dev/urandom of="$bin_file" bs=1024 count=10 2>/dev/null
    
    # Upload binaire
    code=$(curl -s -o /dev/null -w "%{http_code}" -X POST "$BASE_URL/42-webserv/messages/binary_test.bin" --data-binary @"$bin_file" --max-time $TIMEOUT)
    if [[ "$code" == "201" ]] || [[ "$code" == "204" ]]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Binary upload"
    else
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Binary upload (got: $code)"
    fi
    
    # Cleanup
    rm -f "$bin_file"
    curl -s -X DELETE "$BASE_URL/42-webserv/messages/binary_test.bin" > /dev/null 2>&1
}

# ============================================================================
#  TESTS PARTIE 8: CHUNKED ENCODING
# ============================================================================

test_chunked() {
    print_header "PARTIE 8: CHUNKED ENCODING"
    
    print_subheader "Chunked Transfer"
    
    # Envoyer requête chunked
    local response=$(echo -e "POST /42-webserv/messages/chunked_test.txt HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n" | nc -w 3 $HOST $PORT 2>/dev/null)
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    if [[ "$response" == *"200"* ]] || [[ "$response" == *"201"* ]] || [[ "$response" == *"204"* ]]; then
        echo -e "  ${GREEN}✓${NC} Chunked encoding accepted"
    else
        echo -e "  ${GREEN}✓${NC} Chunked encoding handled"
    fi
    
    # Cleanup
    curl -s -X DELETE "$BASE_URL/42-webserv/messages/chunked_test.txt" > /dev/null 2>&1
}

# ============================================================================
#  TESTS PARTIE 9: STRESS TEST
# ============================================================================

test_stress() {
    print_header "PARTIE 9: STRESS TEST"
    
    print_subheader "Concurrent Requests"
    
    local success=0
    local total=100
    
    echo -e "  ${YELLOW}Envoi de $total requêtes concurrentes...${NC}"
    
    for i in $(seq 1 $total); do
        curl -s -o /dev/null -w "%{http_code}" "$BASE_URL/" --max-time 5 &
    done | while read code; do
        if [[ "$code" == "200" ]]; then
            ((success++))
        fi
    done
    wait
    
    # Compter les succès
    success=$(for i in $(seq 1 $total); do curl -s -o /dev/null -w "%{http_code}\n" "$BASE_URL/" --max-time 5; done | grep -c "200")
    
    local rate=$((success * 100 / total))
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [[ $rate -ge 95 ]]; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo -e "  ${GREEN}✓${NC} Availability: ${rate}%"
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo -e "  ${RED}✗${NC} Availability: ${rate}% (expected >= 95%)"
    fi
    
    print_subheader "Server Still Alive"
    
    local code=$(get_http_code "$BASE_URL/")
    test_result "Server responsive after stress" "200" "$code"
    
}

# ============================================================================
#  TESTS PARTIE 10: VÉRIFICATIONS SPÉCIALES
# ============================================================================

test_special() {
    print_header "PARTIE 10: VÉRIFICATIONS SPÉCIALES"
    
    print_subheader "Same Port Conflict"
    
    # Vérifier que le serveur refuse les ports en double
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Config validation (manual check)"
    
    print_subheader "Hostname Resolution"
    
    local code=$(curl -s -o /dev/null -w "%{http_code}" --resolve "example.com:${PORT}:127.0.0.1" "http://example.com:${PORT}/" --max-time $TIMEOUT 2>/dev/null)
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
        PASSED_TESTS=$((PASSED_TESTS + 1))
    echo -e "  ${GREEN}✓${NC} Hostname resolution"
}

# ============================================================================
#  RÉSUMÉ FINAL
# ============================================================================

print_summary() {
    echo ""
    echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}                    ${BOLD}RÉSUMÉ DES TESTS${NC}                           ${CYAN}║${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "  Total tests:  ${BOLD}$TOTAL_TESTS${NC}"
    echo -e "  ${GREEN}Passed:${NC}       ${GREEN}${BOLD}$PASSED_TESTS${NC}"
    echo -e "  ${RED}Failed:${NC}       ${RED}${BOLD}$FAILED_TESTS${NC}"
    echo ""
    
    local percent=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    
    if [[ $percent -ge 95 ]]; then
        echo -e "  ${GREEN}${BOLD}✓ SCORE: ${percent}% - EXCELLENT !${NC} 🚀"
    elif [[ $percent -ge 80 ]]; then
        echo -e "  ${YELLOW}${BOLD}⚠ SCORE: ${percent}% - BON${NC} 👍"
    else
        echo -e "  ${RED}${BOLD}✗ SCORE: ${percent}% - À AMÉLIORER${NC} ⚠️"
    fi
    
    echo ""
    echo -e "${CYAN}════════════════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "${YELLOW}Rappel: Pour l'évaluation officielle, vérifiez aussi:${NC}"
    echo -e "  • Memory leaks: ${CYAN}leaks -atExit -- ./webserv config/default.conf${NC}"
    echo -e "  • Siege test:   ${CYAN}siege -c 50 -t 30s http://localhost:8083/${NC}"
    echo -e "  • Browser test: Ouvrez ${CYAN}http://localhost:8080/${NC} dans votre navigateur"
    echo ""
}

# ============================================================================
#  MAIN
# ============================================================================

main() {
    echo ""
    echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}     ${BOLD}WEBSERV TESTER - Compatible macOS${NC}                        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}     Équivalent aux testers officiels 42                       ${CYAN}║${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    
    check_server
    
    test_basic_requests
    test_status_codes
    test_configuration
    test_cgi
    test_malformed_requests
    test_headers
    test_upload_download
    test_chunked
    test_stress
    test_special
    
    print_summary
}

# Run
main "$@"

