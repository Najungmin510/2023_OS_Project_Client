#include "pch.h"
#include "tipsware.h"

// 컨트롤을 조작했을 때 호출할 함수 만들기
// 컨트롤의 아이디(a_ctrl_id), 컨트롤의 조작 상태(a_notify_code), 선택한 컨트롤 객체(ap_ctrl)
void OnCommand(INT32 a_ctrl_id, INT32 a_notify_code, void* ap_ctrl){
   
    if (a_ctrl_id == 1002) { //id 1002번인 버튼 클릭 시 

        char str[64]; // 선택된 항목의 문자열을 저장할 변수
        void* p_edit = FindControl(1001); // 에디트 컨트롤의 주소를 얻는다.
     
        GetCtrlName(p_edit, str, 64); //문자열 얻은 후 배열에 저장 
        SetCtrlName(p_edit, ""); // 그리고 입력창 초기화
       
        ListBox_InsertString(FindControl(1000), -1, str, 1); //항상 맨 마지막에 추가될 수 있도록 하기
    }
}

// 컨트롤을 조작했을 때 호출할 함수 등록
CMD_MESSAGE(OnCommand)

int main()
{
    ChangeWorkSize(480, 200); //채팅창 크기 설정
    Clear(0, RGB(72, 87, 114)); //배경 색상 변경

    CreateListBox(10, 10, 460, 150, 1000); //사용자가 입력한 채팅을 저장 할 List박스
    CreateEdit(10, 167, 406, 24, 1001, 0); //사용자에게 값을 입력받을 수 있는 EditText 생성
    CreateButton("추가", 420, 164, 50, 28, 1002); //

    ShowDisplay();
    return 0;
}