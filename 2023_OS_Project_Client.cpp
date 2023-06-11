#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

struct AppData { //client에서 내부적으로 사용할 데이터, socket 주소를 관리하기 위해서 만든 구조체
    void* p_socket; //client소켓을 사용하는 객체의 주소
};

const char* quiz_str[20] = { "피아노", "커피", "컴퓨터", "시험지", "떡볶이",
                            "보름달", "껌", "피라미드", "전공책", "머리끈",
                            "크레파스", "달력", "스프링노트", "치킨", "백팩",
                            "에어컨", "텀블러", "얼음", "고등어", "티라미수" };
/*방장에게 랜덤으로 보여줄 단어를 저장한 배열
해당 단어 == 사용자 입력같다면 정답처리 되고 그 사람이 방장이되도록 함*/

void OnServerConnection(void* ap_this, int a_client_index) { //서버 접속 여부 판단하는 기능
    //ap는 소켓 클라이언트 객체, 서버에 접속이 되었는지 안되었는지 알려줌
    //접속성공시 1 아니면 0 반환
    char temp_str[64];
    if (IsConnect(ap_this)) {
        sprintf_s(temp_str, 64, "[맞춰봐]에 오신것을 환영합니다!");
    }
    else {
        sprintf_s(temp_str, 64, "[error] 서버에 접속할 수 없습니다.");
    }
    ListBox_InsertString(FindControl(1000), -1, temp_str);//리스트 박스에 해당 문구를 추가해주기
}

//서버에서 온 데이터를 처리하는 기능
int OnServerMessage(CurrentClientNetworkData* ap_data, void* ap_this, int a_client_index){
    if (ap_data->m_net_msg_id == 1) { //채팅 데이터를 전달 할 아이디를 1로 부여
        ListBox_InsertString(FindControl(1000), -1, ap_data->mp_net_body_data);
        //서버가 전달한 채팅 내용을 리스트박스에 추가 (채팅목록에 추가)
    }
    return 1;
}

//서버와의 접속 상태가 변경되면 관련 알림 문구를 출력하는 기능
void OnCloseUser(void* ap_this, int a_error_flag, int a_client_index) {
    char temp_str[64];

    if (a_error_flag == 1) {
        sprintf_s(temp_str, 64, "서버에서 접속을 해제했습니다."); //서버 측에서 해제시킴
    }
    else { //내가 해제 하였다면
        sprintf_s(temp_str, 64, "로그아웃 하였습니다.");
    }
    ListBox_InsertString(FindControl(1000), -1, temp_str);
}


//서버 접속 시도 기능
void OnConnectBtn(AppData* ap_data) {

    //NULL != 클라이언트 소켓이 존재한다
    //NULL == 클라이언트 소켓이 존재하지 않는다
    if (ap_data->p_socket == NULL) {
        ap_data->p_socket = CreateClientSocket(OnServerConnection, OnServerMessage, OnCloseUser);
        //그래서 서버와 연결할 소켓 객체 하나 생성해주기
    }

    if (!IsConnect(ap_data->p_socket)) { //해당 소켓 객체가 서버와의 접속 상태를 체크
        //접속 x라면 서버와 연결시켜주기
        //서버 아이피는 기본적인걸로.. 
        //서버와 연결하는 것이기에, 내 아이피가 아니라 서버의 아이피를 적어주어야함
        ConnectToServer(ap_data->p_socket, "127.0.0.1", 25001);
    }
}

void OnDisconnectBtn(AppData* ap_data) {
    if (ap_data->p_socket != NULL) { //만약 소켓 클라이언트 객체가 존재한다면
        DeleteClientSocket(ap_data->p_socket); //소켓 클라이언트 객체 제거 , 그럼 연결이 해제
        //메모리만 해제되는거라 null로 따로 초기화 해줘야함!!! 안그럼 ap_data쪽 데이터 사라지지 않음
        ap_data->p_socket = NULL;
    }
}

void SendChatData(AppData* ap_data) {
    void* p_edit = FindControl(1020); //edit 컨트롤, 문자열을 입력받은 컨트롤 아이디 가져오기
    char str[128]; //문자열 저장할 변수

    GetCtrlName(p_edit, str, 128); //str 배열에 사용자가 입력한 데이터 복사
    SetCtrlName(p_edit, ""); //그리고 edit컨트롤 초기화


    if (ap_data->p_socket && IsConnect(ap_data->p_socket)) { //클라이언트 소켓 객체 생성 ok & 서버와 접속 상태 ok라면
        SendFrameDataToServer(ap_data->p_socket, 1, str, strlen(str) + 1);
        //채팅 내용 전송할건데, 아이디 1로 주고 (아이디는 1로 정의를 했었기에 이걸로 맞춰줘야)
        // 소켓 객체 (사용자 누군지 알아야하니까)와 함께 이를 보내줌
    }
}


// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl){

    AppData* p_data = (AppData*)GetAppData(); //프로그램 내부 데이터 주소 가져오기

    //입력버튼 or 엔터키 누를 시에 사용자가 입력한 데이터를 보낼 수 있도록 함
    if (a_ctrl_id == 1013 || (a_ctrl_id == 1020 && a_notify_code == 1000)) {
        SendChatData(p_data);
    } 
    else if (a_ctrl_id == 1011) { //입장 버튼 클릭 시 
        OnConnectBtn(p_data); //서버와 연결하는 함수 호출
    }
    else if (a_ctrl_id == 1012) {
        OnDisconnectBtn(p_data);
    }
}

// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
    AppData data = { NULL }; //프로그램이 내부적으로 사용할 메모리 선언
    SetAppData(&data, sizeof(AppData)); //지정한 변수를 내부 데이터로 사용

    ChangeWorkSize(520, 180); //채팅창 크기 설정
    Clear(0, RGB(41, 22, 77)); //배경 색상 변경
    StartSocketSystem(); //socket 사용 선언

    CreateListBox(10, 36, 500, 100, 1000); // 채팅 목록(사용자가 입력한 채팅을 저장 할 List박스)

    CreateButton("입장", 407, 3, 50, 28, 1011); // 서버[접속]버튼
    CreateButton("나가기", 460, 3, 50, 28, 1012); // 서버연결[해제]버튼
    CreateButton("입력", 460, 140, 50, 28, 1013); // [입력]버튼

    void* p = CreateEdit(10, 143, 446, 24, 1020, 0); //사용자에게 값을 입력받을 수 있는 EditText 생성
    EnableEnterKey(p); //사용자가 EditText입력 후 엔터키 누르면 전송될 수 있도록 함

    SelectFontObject("굴림", 12); //글꼴 설정해주기
    TextOut(15, 16, RGB(200, 255, 200), "내 대화");  //제목 추가

    ShowDisplay();
    return 0;
}