#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

struct AppData { //client에서 내부적으로 사용할 데이터, socket 주소를 관리하기 위해서 만든 구조체
    void* p_socket; //client소켓을 사용하는 객체의 주소
    char is_clicked; //마우스 클릭 여부
    char thick; //선 굵기 정보
    COLORREF color; // 선 색상 정보
    POINT prev_pos; //선 이어 그릴때, 이전 마우스의 위치 저장
};

struct DrawLineData { //사용자의 그림 그리기 정보를 저장, 아이디 2번
    char thick; //선 굵기
    COLORREF color; // 선 색상
    POINT start_pos; //선의 시작 좌표
    POINT end_pos; //선의 끝 좌표
};

/*
struct SelectWord {
    char word;
}; */

/* 랜덤으로 보여줄 단어를 저장한 배열, 유저도 랜덤하게 고를거임
해당 단어 == 사용자 입력같다면 정답처리할것*/
const char* quiz_str[] = { "피아노", "커피", "컴퓨터", "시험지", "떡볶이",
                           "보름달", "껌", "피라미드", "전공책", "머리끈",
                           "크레파스", "달력", "스프링노트", "치킨", "백팩",
                           "에어컨", "텀블러", "얼음", "고등어", "티라미수" };

/* -------------------------------------------------------- */

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
// 클라이언트 -> 서버 -> 클라이언트로 전체 사용자에게 보여주게 해야함
// 그래서 서버에서 데이터 복사해서 사용자 전체에게 보내주는것
// 클라이언트 -> 서버 만 하면 이건 단방향통신임

//사용자에게 단어를 보여주는 기능
void ShowWordUser() {
    char temp_str[128]; //단어 저장할 배열

    AppData* p_app_data = (AppData*)GetAppData();

    int Worddata = rand() % 19; // 0 ~ 20까지의 범위
    const char* Word = quiz_str[Worddata];

    SetTextColor(RGB(255, 242, 0));
    ListBox_InsertString(FindControl(1000), -1, "[알림] : 문제자로 선정되셨습니다. 제시된 단어를 그려주세요!");
    ListBox_InsertString(FindControl(1000), -1, Word);

    sprintf_s(temp_str, 128, Word);
    SendFrameDataToServer(p_app_data ->p_socket,100, temp_str, sizeof(temp_str)); //서버로 선택된 단어 보내기 , 정답 확인해야 하니까, id는 100으로 잡음
}

int OnServerMessage(CurrentClientNetworkData* ap_data, void* ap_this, int a_client_index){

    if (ap_data->m_net_msg_id == 1) { //채팅 데이터를 전달 할 아이디를 1로 부여
        ListBox_InsertString(FindControl(1000), -1, ap_data->mp_net_body_data);
        //서버가 전달한 채팅 내용을 리스트박스에 추가 (채팅목록에 추가)
    }
    else if (ap_data->m_net_msg_id == 2) { // 서버에서 온 id 2번이라면 선 그리기이므로
        DrawLineData* p_line_data = (DrawLineData*)ap_data->mp_net_body_data;

        Line(p_line_data->start_pos.x, p_line_data->start_pos.y, p_line_data->end_pos.x,
            p_line_data->end_pos.y, p_line_data->color, p_line_data->thick);

        ShowDisplay(); //화면 보여주기
    }
    else if (ap_data->m_net_msg_id == 3) {
        Clear(0, RGB(41, 22, 77)); //일일이 삭제하는게 아니라 그냥 배경 다시 원색으로 채워주면 됨!!! 
        ShowDisplay(); //
    } 
    else if (ap_data->m_net_msg_id == 12) { //단어를 보고 그림을 그려야 하는 유저로 선택받았다면
        ShowWordUser(); //해당 listbox에 보내주기 
    }
    return 1;
}

//서버와의 접속 상태가 변경되면 관련 알림 문구를 출력하는 기능
void OnCloseUser(void* ap_this, int a_error_flag, int a_client_index) {
    char temp_str[64];

    if (a_error_flag == 1) {
        sprintf_s(temp_str, 64, "서버에서 접속을 해제했습니다. 재접속 해주세요."); //서버 측에서 해제시킴
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

//채팅 데이터 보내는 기능
void SendChatData(AppData* ap_data) {
    void* p_edit = FindControl(1020); //edit 컨트롤, 문자열을 입력받은 컨트롤 아이디 가져오기
    char str[128]; //문자열 저장할 변수

    GetCtrlName(p_edit, str, 128); //str 배열에 사용자가 입력한 데이터 복사
    SetCtrlName(p_edit, ""); //그리고 edit컨트롤 초기화


    if (ap_data->p_socket && IsConnect(ap_data->p_socket)) { //클라이언트 소켓 객체 생성 ok & 서버와 접속 상태 ok라면
        SendFrameDataToServer(ap_data->p_socket, 1, str, strlen(str) + 1);
        //채팅 내용 전송할건데, 아이디 1로 "내가" 정의하였음 (아이디는 1로 정의를 했었기에 서버도 이걸로 맞춰줘야함)
        // 소켓 객체 (사용자 누군지 알아야하니까)와 함께 이를 보내줌
    }
}

//그림퀴즈 시작 버튼 클릭시 게임 이벤트 신호를 서버로 보내는 기능
//한명이라도 게임 시작하면 게임 시작 알림 전체창에 띄워야함. 매우중요!!!! 
void SendStartGame(AppData* ap_data) {
    
    if (ap_data->p_socket && IsConnect(ap_data->p_socket)) {
        SendFrameDataToServer(ap_data->p_socket, 11, "", 1); //서버로 게임시작 이벤트 신호 보내기
    }
}

//화면에 그려진 선 전체삭제 요청 기능
void OnClearImageBtn(AppData* ap_data) { //
    if (ap_data->p_socket && IsConnect(ap_data->p_socket)) { //서버로 전부 삭제하겠다는 신호 보내주기
        SendFrameDataToServer(ap_data->p_socket, 3, "", 1); //나뿐만 아니라 다른 사용자 모두에게도 보여져야 하기에
    }
}

// 색상 바꾸기를 선택한 경우
void OnChangeColorBtn(AppData* ap_data, INT32 a_notify_code, void* ap_ctrl) {
    if (a_notify_code == LBN_SELCHANGE) { //사용자가 선택한 색상을 프로그램 내부에 저장
        ap_data->color = ListBox_GetItemData(ap_ctrl, ListBox_GetCurSel(ap_ctrl));
    }
}

//선 굵기 목록에서 굵기를 변경할 수 있는 기능
void OnChangeThickBtn(AppData* ap_data, INT32 a_notify_code, void* ap_ctrl) {
    if (a_notify_code == LBN_SELCHANGE) { //사용자가 선택한 굵기를 프로그램 내부에 저장
        ap_data->thick = ListBox_GetItemData(ap_ctrl, ListBox_GetCurSel(ap_ctrl));
    }
}


// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl) { //그냥 이벤트 발생 시 해야 할 행동을 정리해둔거라고 보면 됨

    AppData* p_data = (AppData*)GetAppData(); //프로그램 내부 데이터 주소 가져오기

    //입력버튼 or 엔터키 누를 시에 사용자가 입력한 데이터를 보낼 수 있도록 함
    if (a_ctrl_id == 1013 || (a_ctrl_id == 1020 && a_notify_code == 1000)) {
        SendChatData(p_data);
    }
    else if (a_ctrl_id == 1011) { //입장 버튼 클릭 시 
        OnConnectBtn(p_data); //서버와 연결하는 함수 호출
    }
    else if (a_ctrl_id == 1012) { //접속해제버튼
        OnDisconnectBtn(p_data); //해당함수호출
    }
    else if (a_ctrl_id == 1014) { //그림퀴즈 시작 버튼
        SendStartGame(p_data); //서버 접속 여부 확인 후 신호 보내야함 
    }
    else if (a_ctrl_id == 1001) { // 선 색상 변경
        OnChangeColorBtn(p_data, a_notify_code, ap_ctrl);
    }
    else if (a_ctrl_id == 1002) { // 선 굵기 변경
        OnChangeThickBtn(p_data, a_notify_code, ap_ctrl);
    }
    else if (a_ctrl_id == 1010) { //지우기 버튼을 누른 경우
        OnClearImageBtn(p_data);
    }
}

//선과 관련된 정보를 서버로 전송하는 기능
void SendLineData(AppData* ap_data, POINT a_pos) {
    if (ap_data->p_socket && IsConnect(ap_data->p_socket)) {
        DrawLineData send_data; //순서대로 색 굵기 시작좌표 끝좌표 
        send_data.color = ap_data->color;
        send_data.thick = ap_data->thick;
        send_data.start_pos = ap_data->prev_pos;
        send_data.end_pos = a_pos;
        //서버로 데이터 전송
        SendFrameDataToServer(ap_data->p_socket, 2, &send_data, sizeof(DrawLineData));

    }
}

//마우스 왼쪽 버튼 클릭 시
void OnMouseLeftDown(int a_mixed_key, POINT a_pos) {
    AppData* p_data = (AppData*)GetAppData();
    p_data->prev_pos = a_pos; //시작 좌표 저장
    p_data->is_clicked = 1; //마우스 클릭 on 설정
}

void OnMouseMove(int a_mixed_key, POINT a_pos) {
    AppData* p_data = (AppData*)GetAppData(); //프로그램 내부 주소 가져오기

    if (p_data->is_clicked == 1) { //마우스가 클릭상태라면
        SendLineData(p_data, a_pos); //그림 그리기 관련 데이터를 서버로 전송
        p_data->prev_pos = a_pos; //이어그릴 때를 대비해 시작점을 저장해두기
    }
}

void OnMouseLeftUp(int a_mixed_key, POINT a_pos) { //마우스 왼쪽 클릭 -> 해제했을 때 기능
    AppData* p_data = (AppData*)GetAppData();

    if (p_data->is_clicked == 1) { //클릭이 되어있었다면
        SendLineData(p_data, a_pos); //해당 좌표 기억해두고
        p_data->is_clicked = 0;  //클릭해제
    }
}

MOUSE_CMD_MESSAGE(OnMouseLeftDown, OnMouseLeftUp, OnMouseMove, OnCommand);

//선 굵기 선택시 적용할 기능
void DrawThickItem(int index, char* ap_str, int a_str_len, void* ap_data, int a_is_selected, RECT* ap_rect) {
    if (a_is_selected) { //선택되었다면 색상을 다르게 해서 구분이 갈 수 있도록 할것
        SelectPenObject(RGB(100, 220, 255), 2);
    }
    else {
        SelectPenObject(RGB(62, 77, 104), 2);
    }
    SelectBrushObject(RGB(62, 77, 104)); //선의 형태를 넣어주기 위한 칸 생성
    Rectangle(ap_rect->left + 1, ap_rect->top + 1, ap_rect->right, ap_rect->bottom);

    SelectPenObject(RGB(255, 255, 255), (int)ap_data); //선굵기 설정 & 출력 
    Line(ap_rect->left + 6, ap_rect->top + 8, ap_rect->right - 7, ap_rect->top + 8);
}

//색깔 지정 기능
void DrawColorItem(int index, char* ap_str, int a_str_len, void* ap_data, int a_is_selected, RECT* ap_rect) {
    if (a_is_selected) { //선택된 것이라면 구분될 수 있게 색 다르게 지정
        SelectPenObject(RGB(100, 220, 255), 2);
    }
    else {
        SelectPenObject(RGB(62, 77, 104), 2);
    }
   
     SelectBrushObject((COLORREF)ap_data); //선택한 색깔로 변경
     Rectangle(ap_rect->left + 1, ap_rect->top + 1, ap_rect->right, ap_rect->bottom);
    
}

void AddColorToList(void *ap_list_box) { //색깔 파레트 지정해주기
    COLORREF color_table[20] = {   // 표준 20가지 색상 목록
        RGB(100,100,0), RGB(0,0,255), RGB(0,255,0), RGB(0,255,255), RGB(255,0,0), RGB(255,0,255),
        RGB(255,255,0), RGB(255,255,255), RGB(0,0,128), RGB(0,128,0), RGB(0,128,128),
        RGB(128,0,0), RGB(128,0,128), RGB(128,128,0), RGB(128,128,128), RGB(192,192,192),
        RGB(192,220,192), RGB(166,202,240), RGB(255,251,240), RGB(119,67,66)
    };
    ListBox_SetItemWidth(ap_list_box, 18);

    for (int i = 0; i < 20; i++) { //각 색깔 파레트들 넣어주기
        ListBox_SetItemDataEx(ap_list_box, i, "", color_table[i], 0);
    }
    ListBox_SetCurSel(ap_list_box, 0);
}


// 컨트롤을 조작했을 때 호출할 함수 등록
//CMD_MESSAGE(OnCommand)

int main()
{
    AppData data = { NULL, 0, 2, RGB(0, 0, 0), { 0, 0 } }; //프로그램이 내부적으로 사용할 메모리 선언
    SetAppData(&data, sizeof(AppData)); //지정한 변수를 내부 데이터로 사용

    ChangeWorkSize(900, 900); //채팅창 크기 설정
    Clear(0, RGB(41, 22, 77)); //배경 색상 변경
    StartSocketSystem(); //socket 사용 선언

    CreateListBox(10, 36, 500, 100, 1000); // 채팅 목록(사용자가 입력한 채팅을 저장 할 List박스)

    void* p = CreateListBox(520, 36, 56, 114, 1001, DrawColorItem, LBS_MULTICOLUMN);
    AddColorToList(p);
    for (int i = 0; i < 7; i++) {
        ListBox_SetItemDataEx(p, i, "", i + 1, 0); // 선 굵기 추가
    }
    ListBox_SetCurSel(p, 1); //기본값 각 1번째 위치에 있는것들을 선택

    CreateButton("지우기", 354, 3, 50, 28, 1010);   // [지우기] 버튼 생성
    CreateButton("입장", 407, 3, 50, 28, 1011); // 서버[접속]버튼
    CreateButton("나가기", 460, 3, 50, 28, 1012); // 서버연결[해제]버튼
    CreateButton("입력", 460, 140, 50, 28, 1013); // [입력]버튼
    CreateButton("Demo) 그림퀴즈 시작하기 ", 85, 200, 300, 28, 1014); //그림퀴즈 [시작]버튼 

    p = CreateEdit(10, 143, 446, 24, 1020, 0); //사용자에게 값을 입력받을 수 있는 EditText 생성
    EnableEnterKey(p); //사용자가 EditText입력 후 엔터키 누르면 전송될 수 있도록 함

    SelectFontObject("굴림", 12); //글꼴 설정해주기
    TextOut(15, 16, RGB(200, 255, 200), "내 대화");  //제목 추가
    TextOut(525, 16, RGB(200, 255, 200), "선 색상");  // 선 색상 선택 리스트 박스의 제목을 출력
    TextOut(591, 16, RGB(200, 255, 200), "선 굵기");  // 선 굵기 선택 리스트 박스의 제목을 출력
    ShowDisplay();

    return 0;
}