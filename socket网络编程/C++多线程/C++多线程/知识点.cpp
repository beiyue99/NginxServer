#define _CRT_SECURE_NO_WARNINGS 1


//  joinable �ж��Ƿ���detach��join    ������ͼ�






//defer_lock��ǰ��Ϊû���� �����ǳ�ʼ��һ��û�����Ļ�����
//���� unique_lock<mutex> sbguard1(my_mutex1,defer_lock)   �������defer_lockĬ����Ϊ��ֱ������
// �ٵ���  sbguard1.lock()   �����װ��lock�������Լ����������Ժ�������Ҫ�ֶ�unlock
//����Ҳ��װ��unlock�������Ա�����Ĳ�������

//unique_lock��release����ʹ����֮�󶨵Ļ��������룬�����ʱ�û�������������״̬����Ҫ�����ֶ�unlock

//std::unique_lock::try_lock
//std::try_to_lock
// 
// std::mutex mtx;





//{
//    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
//    if (lock.owns_lock()) {
//        // ����������ɹ�������ִ����������
//        // ... ������ִ��һЩ��Ҫ�����Ĵ��� ...
//    }
//    else {
//        // ���������û�б�������ִ����������
//        // ... �����ﴦ����������������� ...
//    }
//} // ��������������������������ﱻ�Զ�����






//std::mutex mtx;
//
//{
//    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
//    // �����mtxû�б�����
//
//    if (lock.try_lock()) {
//        // ����������������������ͻ�ִ����������
//        // ... ������ִ��һЩ��Ҫ�����Ĵ��� ...
//    }
//    else {
//        // ������������������������ͻ�ִ����������
//        // ... �����ﴦ����������������� ...
//    }
//} // ��������������������������ﱻ�Զ�����
